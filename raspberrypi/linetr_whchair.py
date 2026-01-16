from picamera2 import Picamera2
import cv2
import numpy as np
import time
import serial

ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
time.sleep(2)

picam2 = Picamera2()
preview_config = picam2.create_preview_configuration(
    main={"format": "XRGB8888", "size": (320, 240)}
)
picam2.configure(preview_config)
picam2.set_controls({"AwbEnable": True})
picam2.start()
time.sleep(1)

color_ranges = {
    "RED": (np.array([0, 100, 50]), np.array([10, 255, 255])),
    "BLUE": (np.array([90, 150, 50]), np.array([140, 255, 255])),
    "YELLOW": (np.array([15, 80, 80]), np.array([35, 255, 255]))
}

selected_color = "RED"
back_sent = False
back_start_time = None
BACK_DURATION = 0.3
miss_count = 0
MISS_THRESHOLD = 3
last_direction = None

while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        if line in color_ranges.keys():
            selected_color = line
            print(f"Selected color changed to: {selected_color}")

    frame = picam2.capture_array()
    frame = cv2.flip(frame, -1)

    height, width, _ = frame.shape
    roi = frame[0:int(height*3/4), 0:width]

    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    direction = "STOP"

    
    if selected_color == "RED":
        direction = "FORWARD"
    else:
        lower, upper = color_ranges[selected_color]
        mask = cv2.inRange(hsv, lower, upper)
        contours, _ = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if len(contours) > 0:
            c = max(contours, key=cv2.contourArea)
            M = cv2.moments(c)
            if M['m00'] != 0:
                cx = int(M['m10'] / M['m00'])
                left_boundary = width // 3
                right_boundary = width * 2 // 3

                if cx < left_boundary:
                    direction = "LEFT"
                elif cx > right_boundary:
                    direction = "RIGHT"
                else:
                    direction = "FORWARD"

                miss_count = 0
                back_sent = False
                back_start_time = None
        else:
            miss_count += 1
            if miss_count >= MISS_THRESHOLD:
                direction = "BACK"
            else:
                direction = "STOP"

    
    if direction == "BACK":
        cv2.putText(frame, f"{selected_color} - {direction}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    else:
        cv2.putText(frame, f"{selected_color} - {direction}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    if direction != last_direction and direction != "BACK":
        ser.write(("STOP\n").encode('utf-8'))
        time.sleep(0.1)
    last_direction = direction

    ser.write((direction + "\n").encode('utf-8'))

    cv2.imshow("Line Tracing", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()
