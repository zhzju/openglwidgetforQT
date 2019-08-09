# openglwidgetforQT
<h1>An extended opengl widget class for QT.</h1>
<div>
    这是一个基于Qt中OpenGLWidget类的一个扩展类，实现了基于opengl的利用鼠标对图像放大、缩小、拖动等变换。
</div>
<ul>
    <li>利用鼠标左键可以实现对图片在2D平面上自由拖动</li>
    <li>利用鼠标滚轮可以实现对图片进行缩放，缩放倍数由zoomRatioMax及zoomRatioMin两个参数决定</li>
    <li>利用方法setgrayback及setblackback可实现将图片设置为全黑或全黑</li>
    <li>内置mousepressevent，可结合Qt实现对鼠标点击的监听</li>
    <li>通过控制标志位可实现图片边框的配置</li>
    <li>内置标框功能，可利用鼠标左键点击实现对图片的标记，并在标记结束后会发出相关信号，具体看源码</li>
</ul>