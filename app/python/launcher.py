import time
import math
import subprocess
from pynput import mouse

gas = 0
brake = 0
position = 0
isPressed = False
def on_scroll(x, y, dx, dy):
    #print('Scrolled {0} at {1}'.format('down' if dy < 0 else 'up',(x, y)))
    pass


def on_click(x, y, button, pressed):
    print('{0} at {1}'.format(
        'Pressed' if pressed else 'Released',
        (x, y)))
    if pressed:
        global isPressed
        isPressed = not isPressed

def on_move(x, y):
    #print('Pointer moved to {0}'.format((x, y)))
    global gas, brake
    gas = math.trunc((1 - y / 1080) * 1000) / 1000
    brake = math.trunc((x / 1920) * 1000) / 1000
    if gas < 0:
        gas = 0
    if gas > 1:
        gas = 1
    if brake < 0:
        brake = 0
    if brake > 1:
        brake = 1

process = subprocess.Popen(
    ['../build/telemetry_app.exe'], 
    stdin=subprocess.PIPE, 
    #stdout=subprocess.PIPE, 
    text=True
)

with mouse.Listener(on_click=on_click, on_scroll=on_scroll, on_move=on_move) as listener:
    try:
        while True:
            process.stdin.write(str(f"{gas} {brake}\n"))
            process.stdin.flush()
            time.sleep(0.02)
            
    except KeyboardInterrupt:
        print("Exiting...")
        process.stdin.close()
        process.terminate()
        process.wait()

