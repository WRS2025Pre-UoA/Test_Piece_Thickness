import cv2
import pytesseract
import numpy as np
import re

# Tesseractのパス（環境に応じて調整）
pytesseract.pytesseract.tesseract_cmd = r'/usr/bin/tesseract'  # Windowsなら例: 'C:\\Program Files\\Tesseract-OCR\\tesseract.exe'

# 画像の読み込み
image = cv2.imread('IMG_6.png')

# グレースケール変換
gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

# 数字部分の領域をクロップ
cropped = gray[75:170, 60:380]

# 画像の前処理（しきい値処理 + ノイズ除去）
blurred = cv2.GaussianBlur(cropped, (3, 3), 0)
_, thresh = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

# OCRで数字読み取り
custom_config = r'--psm 6 -c tessedit_char_whitelist=0123456789.'
text = pytesseract.image_to_string(thresh, config=custom_config)
digits = re.findall(r'\d+\.\d+|\d+', text)  # 小数 or 整数にマッチ
if digits:
    print("🔢 数字抽出結果:", digits[0])
else:
    print("❌ 認識できた数字が見つかりませんでした")

# 確認用に表示
cv2.imshow("cropped", cropped)
cv2.imshow("thresh", thresh)
cv2.waitKey(0)
cv2.destroyAllWindows()
