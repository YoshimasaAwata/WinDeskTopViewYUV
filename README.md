# WinDeskTopViewYUV
Direct2Dを使用した、YUV⇒RGB変換、動画表示のサンプルコードです。  
Visual Studio 2022を使用しています。  

YUVのファイルは  
http://trace.eas.asu.edu/yuv/  
のCIFファイルを使用してください。

詳細な説明は  
[YUV⇒RGB変換(Direct2D編)](https://yoshia.mydns.jp/programming/programming-tips/yuvrgb-conversion/direct2d-part/)  
を参照してください。

tagは以下の章に対応しています。

- direct2dsample: プログラム作成  
- addshowyuv: 3. YUV表示プログラムの作成 ⇒ 3.4. 確認
- show1frame: 3. YUV表示プログラムの作成 ⇒ 3.5. 1フレームを表示
- playvideo: 4. 動画プログラムの作成

なお、masterについては、DirectXMathによる高速化に対応しています。