import cv2
from pyzbar.pyzbar import decode
import pyttsx3
import winsound

def speak_text(text):
    engine = pyttsx3.init()
    engine.setProperty("rate", 150)

    # Female voice selection
    voices = engine.getProperty("voices")
    for v in voices:
        if "zira" in v.name.lower() or "female" in v.name.lower():
            engine.setProperty("voice", v.id)
            break

    engine.say(text)
    engine.runAndWait()
    engine.stop()


cap = cv2.VideoCapture(1)

print("Scanning for QR codes... Press 'q' to quit.")

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    codes = decode(frame)
    for code in codes:
        qr_text = code.data.decode("utf-8")

        # Create formatted message
        formatted_text = f"Order Number {qr_text} Please collect your items"

        # Draw bounding box
        pts = [(p.x, p.y) for p in code.polygon]
        for i in range(len(pts)):
            cv2.line(frame, pts[i], pts[(i + 1) % len(pts)], (0, 255, 0), 2)

        # Display formatted text above QR region
        cv2.putText(frame, formatted_text, (pts[0][0], pts[0][1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        # Beep every detection
        winsound.Beep(1000, 200)

        # Print formatted message
        print(formatted_text)

        # Speak formatted message
        speak_text(formatted_text)

    cv2.imshow("QR Scanner (Press q to quit)", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()