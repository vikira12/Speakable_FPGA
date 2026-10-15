#include "speakable_fpga.h"
#include "xspeakable_cnn_top.h"
#include <string.h>

// IP의 DDR 배치 (speakable_hls_src/speakable_types.h와 같아야 함)
#define MAX_C       64
#define MAX_H       64
#define MAX_W       64
#define OC_SLOT     (MAX_C * 9)          // 출력 채널 하나의 가중치 칸 (576바이트)
#define LAYER_CONV  0
#define LAYER_DW    1
#define LAYER_PW    2

#define FM_SIZE     (MAX_C * MAX_H * MAX_W)
#define WP_SIZE     (MAX_C * OC_SLOT)

static XSpeakable_cnn_top cnn;

// 가속기가 읽고 쓰는 버퍼 (CPU 주소). 가속기에는 dev_addr()로 바꾼 물리 주소를 준다
static int8_t  *fm[2];
static int8_t  *wpack[SPK_FPGA_LAYERS];
static int32_t *bias[SPK_FPGA_LAYERS];
static int32_t *mult[SPK_FPGA_LAYERS];
static int32_t *shift[SPK_FPGA_LAYERS];
static int8_t  dense[FM_SIZE];          // 패딩 배치 → 실제 크기 배치 변환용 (일반 메모리)

#ifdef __linux__
// ---------------------------------------------------------------- Linux (PetaLinux, UIO)
// 장치 트리의 generic-uio 노드: map0 = AXI-Lite 레지스터 (드라이버가 매핑), map1 = 예약된 DDR 버퍼.
// map1은 캐시를 거치지 않는 매핑이라 flush·invalidate가 필요 없다. 대신 정렬 안 된 접근이 버스 오류가 될 수 있어
// 바이트 단위로 읽고 쓰고, 가속기 시작 전·완료 후에 메모리 장벽을 둔다
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define UIO_NAME "speakable_cnn_top"                  // 장치 트리 노드 이름 (커널에 따라 "@40000000"이 붙음)

static volatile uint8_t *buf_virt;
static uint64_t buf_phys;
static size_t buf_size;

static int read_hex(const char *path, unsigned long long *v) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    const int ok = fscanf(f, "%llx", v) == 1;
    fclose(f);
    return ok ? 0 : -1;
}

// /sys/class/uio에서 이름이 UIO_NAME으로 시작하는 장치 → 번호, 정확한 이름 (드라이버 초기화에 그대로 넘김)
static int find_uio(char *name, size_t len) {
    DIR *d = opendir("/sys/class/uio");
    if (!d) return -1;
    struct dirent *e;
    int found = -1;
    char path[320];
    while ((e = readdir(d)) != NULL) {
        if (strncmp(e->d_name, "uio", 3) != 0) continue;
        snprintf(path, sizeof(path), "/sys/class/uio/%s/name", e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        const int ok = fgets(name, (int)len, f) != NULL;
        fclose(f);
        if (ok && strncmp(name, UIO_NAME, strlen(UIO_NAME)) == 0) {
            name[strcspn(name, "\n")] = 0;
            found = atoi(e->d_name + 3);
            break;
        }
    }
    closedir(d);
    return found;
}

// UIO 장치의 map1(DDR 버퍼)을 매핑
static int map_buffers(int uio) {
    char path[320];
    unsigned long long addr, size;
    snprintf(path, sizeof(path), "/sys/class/uio/uio%d/maps/map1/addr", uio);
    if (read_hex(path, &addr)) return -1;
    snprintf(path, sizeof(path), "/sys/class/uio/uio%d/maps/map1/size", uio);
    if (read_hex(path, &size)) return -1;
    snprintf(path, sizeof(path), "/dev/uio%d", uio);
    const int fd = open(path, O_RDWR | O_SYNC);
    if (fd < 0) return -1;
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 1 * getpagesize());   // map1
    close(fd);
    if (p == MAP_FAILED) return -1;
    buf_virt = (volatile uint8_t *)p;
    buf_phys = addr;
    buf_size = size;
    return 0;
}

static uint64_t dev_addr(const void *p) { return buf_phys + (uint64_t)((const volatile uint8_t *)p - buf_virt); }

static void dev_write(void *dst, const void *src, size_t n) {
    volatile uint8_t *d = (volatile uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
}

static void dev_read(void *dst, const void *src, size_t n) {
    const volatile uint8_t *s = (const volatile uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
}

static void dev_zero(void *dst, size_t n) {
    volatile uint8_t *d = (volatile uint8_t *)dst;
    for (size_t i = 0; i < n; i++) d[i] = 0;
}

static void dev_to_device(const void *p, size_t n) { (void)p; (void)n; __sync_synchronize(); }
static void dev_from_device(const void *p, size_t n) { (void)p; (void)n; __sync_synchronize(); }

static int platform_init(uintptr_t base_addr) {
    (void)base_addr;                                    // Linux에서는 UIO 이름으로 찾음
    char name[64];
    const int uio = find_uio(name, sizeof(name));
    if (uio < 0 || XSpeakable_cnn_top_Initialize(&cnn, name) != XST_SUCCESS) return -1;
    if (map_buffers(uio) != 0) return -2;
    // 64바이트 정렬로 버퍼 배치: 특징맵 2개, 레이어별 가중치·bias·mult·shift
    size_t off = 0;
#define TAKE(ptr, type, bytes) do { ptr = (type *)(buf_virt + off); off += ((bytes) + 63) & ~(size_t)63; } while (0)
    TAKE(fm[0], int8_t, FM_SIZE);
    TAKE(fm[1], int8_t, FM_SIZE);
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        TAKE(wpack[l], int8_t, WP_SIZE);
        TAKE(bias[l], int32_t, MAX_C * 4);
        TAKE(mult[l], int32_t, MAX_C * 4);
        TAKE(shift[l], int32_t, MAX_C * 4);
    }
#undef TAKE
    return off <= buf_size ? 0 : -3;
}

#else
// ---------------------------------------------------------------- 베어메탈 (standalone)
// 버퍼는 정적 배열 (CPU 주소 = 물리 주소). 가속기는 캐시를 거치지 않고 DDR을 직접 읽고 쓰므로
// CPU가 쓴 뒤 flush, 가속기 결과를 읽기 전 invalidate
#include "xil_cache.h"

static int8_t  fm_buf[2][FM_SIZE]                __attribute__((aligned(64)));
static int8_t  wpack_buf[SPK_FPGA_LAYERS][WP_SIZE] __attribute__((aligned(64)));
static int32_t bias_buf[SPK_FPGA_LAYERS][MAX_C]  __attribute__((aligned(64)));
static int32_t mult_buf[SPK_FPGA_LAYERS][MAX_C]  __attribute__((aligned(64)));
static int32_t shift_buf[SPK_FPGA_LAYERS][MAX_C] __attribute__((aligned(64)));

static uint64_t dev_addr(const void *p) { return (uint64_t)(UINTPTR)p; }
static void dev_write(void *dst, const void *src, size_t n) { memcpy(dst, src, n); }
static void dev_read(void *dst, const void *src, size_t n) { memcpy(dst, src, n); }
static void dev_zero(void *dst, size_t n) { memset(dst, 0, n); }
static void dev_to_device(const void *p, size_t n) { Xil_DCacheFlushRange((UINTPTR)p, n); }
static void dev_from_device(const void *p, size_t n) { Xil_DCacheInvalidateRange((UINTPTR)p, n); }

static int platform_init(uintptr_t base_addr) {
    if (XSpeakable_cnn_top_Initialize(&cnn, base_addr) != XST_SUCCESS) return -1;
    for (int i = 0; i < 2; i++) fm[i] = fm_buf[i];
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        wpack[l] = wpack_buf[l];
        bias[l] = bias_buf[l];
        mult[l] = mult_buf[l];
        shift[l] = shift_buf[l];
    }
    return 0;
}
#endif

// ---------------------------------------------------------------- 공통

static int ip_type(spk_layer_type_t t) {
    return t == SPK_DW3X3 ? LAYER_DW : (t == SPK_PW1X1 ? LAYER_PW : LAYER_CONV);
}

int spk_fpga_init(uintptr_t base_addr) {
    const int err = platform_init(base_addr);
    if (err != 0) return err;

    // 일반 메모리에서 IP 배치로 만든 뒤 가속기 버퍼로 복사 (shift는 모델에서 int8, IP에서는 채널당 int32)
    static int8_t slot[WP_SIZE];
    static int32_t b32[MAX_C], m32[MAX_C], s32[MAX_C];
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        const spk_layer_t *L = &spk_layers[l];
        const int per_oc = L->type == SPK_CONV3X3 ? L->in_c * 9 : (L->type == SPK_DW3X3 ? 9 : L->in_c);
        memset(slot, 0, sizeof(slot));
        memset(b32, 0, sizeof(b32));
        memset(m32, 0, sizeof(m32));
        memset(s32, 0, sizeof(s32));
        for (int oc = 0; oc < L->out_c; oc++) {
            memcpy(&slot[oc * OC_SLOT], L->weight + oc * per_oc, per_oc);
            b32[oc] = L->bias[oc];
            m32[oc] = L->mult[oc];
            s32[oc] = L->shift[oc];
        }
        dev_write(wpack[l], slot, WP_SIZE);
        dev_write(bias[l], b32, sizeof(b32));
        dev_write(mult[l], m32, sizeof(m32));
        dev_write(shift[l], s32, sizeof(s32));
        dev_to_device(wpack[l], WP_SIZE);
        dev_to_device(bias[l], MAX_C * 4);
        dev_to_device(mult[l], MAX_C * 4);
        dev_to_device(shift[l], MAX_C * 4);
    }
    for (int i = 0; i < 2; i++) {
        dev_zero(fm[i], FM_SIZE);
        dev_to_device(fm[i], FM_SIZE);
    }
    return 0;
}

// 패딩 배치 [MAX_C][MAX_H][MAX_W] (가속기 버퍼) → 실제 크기 [C][H][W]
static void unpad(const int8_t *src, int c, int h, int w, int8_t *dst) {
    for (int i = 0; i < c; i++)
        for (int y = 0; y < h; y++)
            dev_read(&dst[(i * h + y) * w], &src[(i * MAX_H + y) * MAX_W], w);
}

static void run_layer(int l, const int8_t *in, int8_t *out) {
    const spk_layer_t *L = &spk_layers[l];
    XSpeakable_cnn_top_Set_ifmap(&cnn, dev_addr(in));
    XSpeakable_cnn_top_Set_ofmap(&cnn, dev_addr(out));
    XSpeakable_cnn_top_Set_weight(&cnn, dev_addr(wpack[l]));
    XSpeakable_cnn_top_Set_bias(&cnn, dev_addr(bias[l]));
    XSpeakable_cnn_top_Set_mult(&cnn, dev_addr(mult[l]));
    XSpeakable_cnn_top_Set_shift(&cnn, dev_addr(shift[l]));
    XSpeakable_cnn_top_Set_in_h(&cnn, L->in_h);
    XSpeakable_cnn_top_Set_in_w(&cnn, L->in_w);
    XSpeakable_cnn_top_Set_in_ch(&cnn, L->in_c);
    XSpeakable_cnn_top_Set_out_ch(&cnn, L->out_c);
    XSpeakable_cnn_top_Set_layer_type(&cnn, ip_type(L->type));
    XSpeakable_cnn_top_Set_stride(&cnn, L->stride);
    XSpeakable_cnn_top_Set_do_relu(&cnn, L->relu);
    XSpeakable_cnn_top_Set_do_pool(&cnn, 0);
    XSpeakable_cnn_top_Start(&cnn);
    while (!XSpeakable_cnn_top_IsDone(&cnn)) ;
}

void spk_fpga_cnn_run(const int8_t *input, int8_t *embedding, spk_layer_hook_t hook, void *ctx) {
    // 입력 1×40×64를 패딩 배치로 (행 폭이 64라 행 단위 복사)
    for (int y = 0; y < SPK_IN_H; y++) dev_write(&fm[0][y * MAX_W], &input[y * SPK_IN_W], SPK_IN_W);
    dev_to_device(fm[0], SPK_IN_H * MAX_W);

    int cur = 0;
    for (int l = 0; l < SPK_FPGA_LAYERS; l++) {
        run_layer(l, fm[cur], fm[1 - cur]);
        cur = 1 - cur;
        if (hook) {
            const spk_layer_t *L = &spk_layers[l];
            dev_from_device(fm[cur], L->out_c * MAX_H * MAX_W);
            unpad(fm[cur], L->out_c, L->out_h, L->out_w, dense);
            hook(l + 1, dense, L->out_c * L->out_h * L->out_w, ctx);
        }
    }

    // L9 출력(64×5×8) → ARM에서 GAP, FC
    const spk_layer_t *L9 = &spk_layers[SPK_FPGA_LAYERS - 1];
    dev_from_device(fm[cur], L9->out_c * MAX_H * MAX_W);
    unpad(fm[cur], L9->out_c, L9->out_h, L9->out_w, dense);
    int8_t gap_out[SPK_EMB_DIM];
    spk_run_layer(10, dense, gap_out);
    if (hook) hook(10, gap_out, SPK_EMB_DIM, ctx);
    spk_run_layer(11, gap_out, embedding);
    if (hook) hook(11, embedding, SPK_EMB_DIM, ctx);
}
