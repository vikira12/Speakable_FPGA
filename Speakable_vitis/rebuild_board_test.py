"""플랫폼 하드웨어 갱신(Vivado xsa) → 플랫폼·보드 앱 빌드

사용 (Speakable_vitis에서): vitis -s rebuild_board_test.py [fpga|frontend]
  fpga     (기본) speakable_fpga_test.c: FPGA 가속 추론 레이어별 비트 비교 + ARM 단독 대비 시간
  frontend speakable_frontend_test.c: PCM → 전처리 → CNN(ARM·FPGA), 단계별 시간 (SPK_ON_BOARD)
  인자 대신 환경 변수 SPK_BOARD_TEST로도 고를 수 있다
보드 앱(app_component)은 speakable_arm_ref/src의 검증 프로그램과 generated/ 헤더를 직접 빌드한다
(UserConfig.cmake의 소스 목록·컴파일 정의를 이 스크립트가 고쳐 쓴다). 결과: app_component/build/app_component.elf
앱 실행 설정(launch.json)은 app_component/_ide/의 비트스트림·ps7_init 복사본을 쓰므로 새 플랫폼 것으로 바꿔 둔다
(안 바꾸면 이전 하드웨어가 올라감)
"""
import os
import re
import shutil
import sys
import vitis

WS = os.path.dirname(os.path.abspath(__file__))
XSA = os.path.join(WS, "..", "Speakable_vivado", "speakable_vivado", "speakable_bd_wrapper.xsa")
SRC = '"${CMAKE_SOURCE_DIR}/../../../speakable_arm_ref/src/'
TESTS = {
    "fpga": (["speakable_fpga_test.c", "speakable_fpga.c", "speakable_model.c"], ""),
    "frontend": (["speakable_frontend_test.c", "speakable_frontend.c", "speakable_fpga.c", "speakable_model.c"], "SPK_ON_BOARD"),
}


def set_test(name):
    """UserConfig.cmake의 USER_COMPILE_SOURCES·USER_COMPILE_DEFINITIONS를 고른 검증 프로그램으로 (CRLF 유지)"""
    sources, define = TESTS[name]
    p = os.path.join(WS, "app_component", "src", "UserConfig.cmake")
    s = open(p, "rb").read().decode("utf8").replace("\r\n", "\n")
    s = re.sub(r"set\(USER_COMPILE_SOURCES\n.*?\n\)", "set(USER_COMPILE_SOURCES\n" +
               "\n".join(SRC + f + '"' for f in sources) + "\n)", s, count=1, flags=re.S)
    s = re.sub(r"set\(USER_COMPILE_DEFINITIONS\n.*?\n\)", f'set(USER_COMPILE_DEFINITIONS\n"{define}"\n)', s, count=1, flags=re.S)
    open(p, "wb").write(s.replace("\n", "\r\n").encode("utf8"))


args = [a for a in sys.argv[1:] if a in TESTS]
test = args[0] if args else os.environ.get("SPK_BOARD_TEST", "fpga")
if test not in TESTS:
    sys.exit(f"검증 프로그램은 {sorted(TESTS)} 중 하나")
set_test(test)
print("보드 검증 프로그램:", test)

client = vitis.create_client()
client.set_workspace(path=WS)
platform = client.get_component(name="speakable_platform")
print("플랫폼 하드웨어 갱신:", os.path.abspath(XSA))
platform.update_hw(hw_design=os.path.abspath(XSA))
platform.build()
hw = os.path.join(WS, "speakable_platform", "export", "speakable_platform", "hw", "sdt")
for src, dst in (("speakable_bd_wrapper.bit", "bitstream"), ("ps7_init.tcl", "psinit")):
    os.makedirs(os.path.join(WS, "app_component", "_ide", dst), exist_ok=True)
    shutil.copy(os.path.join(hw, src), os.path.join(WS, "app_component", "_ide", dst, src))
print("앱 실행용 비트스트림·ps7_init 갱신")
app = client.get_component(name="app_component")
app.build()
vitis.dispose()
