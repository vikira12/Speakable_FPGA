"""플랫폼 하드웨어 갱신(Vivado xsa) → 플랫폼·보드 앱 빌드

사용 (Speakable_vitis에서): vitis -s rebuild_board_test.py
보드 앱(app_component)은 speakable_arm_ref/src의 FPGA 검증 프로그램과 generated/ 헤더를 직접 빌드한다
(UserConfig.cmake). 결과: app_component/build/app_component.elf
앱 실행 설정(launch.json)은 app_component/_ide/의 비트스트림·ps7_init 복사본을 쓰므로 새 플랫폼 것으로 바꿔 둔다
(안 바꾸면 이전 하드웨어가 올라감)
"""
import os
import shutil
import vitis

WS = os.path.dirname(os.path.abspath(__file__))
XSA = os.path.join(WS, "..", "Speakable_vivado", "speakable_vivado", "speakable_bd_wrapper.xsa")

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
