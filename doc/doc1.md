## 真实感海洋的绘制（一）：基于统计学模型的水面模拟方法详解

学习了基本的OpenGL和图形学知识后，第一个想做的事情就是画水（笑），因为对我而言各种游戏里面往往最令人印象深刻的就是那波光粼粼、使人心旷神怡的海面了~当然，海面的模拟并不是一件简单的事情TAT…因此决定对于其中较为一些复杂的内容整理出来发在博客上，供以后参考。

###一、问题描述
首先出于OpenGL编程习惯，约定$y$轴正方向表示垂直向上，$xz$平面表示水平面。那么符合直觉的水面描述方式是建立一个连续的高度场，使得这个高度场的图形尽可能接近于水面：
$$
y=H(x,z,t)
$$

其中，$t$是时间变量，因为我们希望水面随着时间而自然地改变。

当然，对于计算机而言必须用离散的方式来近似地存储这一高度场。计算机图形最常见的内部存储方式是，通过由三维顶点坐标集合$V=\{v_1,v_2,…,v_n\},v=(x,y,z)$及一些顶点间连接关系集合$E$描述的一个三角形集合$T=<V,E>$。对于海面这个特定问题而言，根据顶点间的坐标可以自动生成顶点和相邻顶点的连接关系，例如如果我们设置顶点为
$$
V=\{(0.1k,0,0.1k)|k\in Z \ \mbox{and}\  k\in [-255,255]\}
$$
一个顶点和相邻8个顶点之间构成4个正方形，每个正方形用两个三角形就可以描述顶点的连接关系，因此我们只用考虑顶点集合$V$。不妨设一开始对$\forall  v\in V$，$ v_y=0$，那么我们的任务就是对任意给定的顶点$v$，根据这个顶点的水平坐标$(x,y)$求出$v_y$即可。

这样，这个问题形式化描述和计算机的实现方案就形成了。

![我自己的Gestner波实现（Shader随便写的）](https://images2018.cnblogs.com/blog/1322089/201803/1322089-20180311141708763-943979286.gif)

###二、一些简单方法

只要你学过高中数学和物理，很容易就能想到使用三角函数来模拟一条水面波，多个随机的三角函数叠加就能产生一个“水面”，然而基于三角函数的水面往往显得很不真实，因为这个猜想基于水面波是简谐运动的模型，这个模型似乎简化过头了。在流体力学中，有一种波是某个理想模型下的流体微分方程的近似解，被称作Gestner波，感兴趣的读者可以查阅相关资料。Gestner波最大的优势是实现简单、容易计算，并且看上去足够逼真，是性价比很高的解决方案，在很多游戏中得到了广泛应用。缺点是让效果好看需要大量的调参工作，并且依然是高度简化的理想模型，与真实情况还有不小的差距。

### 三、统计学模型：形式化描述

那么如果我们不计性能要绘制出最为逼真的海面呢？我们需要更为复杂的简化程度更低的模型。这里就要不得不提著名的论文“Simulating Ocean Water"[1]中提出的统计学模型（据说是海洋科学用于描述海洋的模型）。由于这个模型相当复杂，因此本文的核心目的就是讲清楚这个模型是什么，并提供一种最简单的计算方法。

为了便于后续描述，我们把上面的高度场的形式变化一下
$$
y=H(x,z,t)=H(\vec{x}, t)\mbox{, where }\vec{x}=(x,z)
$$
那么这个模型的形式化描述是
$$
H(\vec{x},t)=\sum_{\vec{k} }
h(\vec{k},t)e^{ i \vec{k} \cdot \vec{x}} 
\mbox{, where } \vec{k}=(k_x,k_z)=(\frac{2\pi n}{L_x},\frac{2\pi m}{L_z})
$$

其中，$e$是自然对数，$i$是虚数单位，并且
$$
-\frac{N}{2}\le n<\frac{N}{2},-\frac{M}{2}\le m < \frac{M}{2}\\
e^{i\vec{k} \cdot \vec{x}} = \cos(\vec{k} \cdot \vec{x}) +i\sin(\vec{k} \cdot \vec{x})\\
h(\vec{k},t)=h_0(\vec{k})e^{i\omega (\vec{k})t}+
h_0^*(-\vec{k})e^{-i\omega(\vec{k})t}
\mbox{, where }\omega(\vec{k})=\sqrt{g|\vec{k}|}\mbox{, * 表示共轭}\\
h_0(\vec{k})=\frac{1}{\sqrt{2}}(\xi_r+i\xi_i)\sqrt{P_h({\vec{k}})} \\
P_h(\vec{k})=A\frac{e^{-1/(|\vec k|L)^2}}{|\vec k|^4}|\hat{k}\cdot\hat{D_{w}}|^2\\
L=\frac{V_w^2}{g}
$$

在这“坨”公式中，我们需要控制的变量有：

1. 水面的大小$L_x,L_z$.
2. 顶点网格的密度$N,M$.
2. 风的方向$\vec{D_{w}}$和风的速度$V_w$.
3. 波涛汹涌的幅度$A$.
4. 符合正态分布的两个独立随机变量$\xi_r$和$\xi_i$，均值为$0$，标准差为$1$.

此外，还需要一个随机的偏置向量$\vec{D}(\vec{x},t)$来模拟海水的随机扰动，可以如下设置
$$
\vec{D}(\vec{x},t)=\sum_{\vec{k}}-i\frac{\vec{k}}{|\vec{k}|}
h(\vec{k},t)e^{i\vec{k}\cdot \vec{x}}
$$
当然，为了便于后续光照计算，我们还需要法向量$\vec{N}(\vec{x},t)$
$$
\epsilon(\vec{x},t)=\nabla H(\vec{x},t)=\sum_{\vec{k}}i\vec{k}h(\vec{k},t)e^{i\vec{k}\cdot \vec{x}}\\
\begin{align}
\vec{N}(\vec{x},t)&=(0,1,0)-(\epsilon_x(\vec{x},t),0,\epsilon_z(\vec{x},t))\\
&=(-\epsilon_x(\vec{x},t),1,-\epsilon_z(\vec{x},t))
\end{align}
$$
暴力实现对于$N\times N$水面上对每一个顶点做高度计算的复杂度为$O(n^4)$，这个效率是非常低的，因此我们需要对其效率做出改进才能成为真正实用的算法。

### 五、实现结果

（待更）

### 参考文献

1. Tessendorf, Jerry. Simulating Ocean Water. *In SIGGRAPH 2002 Course Notes #9 (Simulating Nature: Realistic and Interactive Techniques)*, ACM Press.
2. Keith Lantz. Ocean Simulating Part One: Using the Discrete Fourier Transform. https://www.keithlantz.net/2011/10/ocean-simulation-part-one-using-the-discrete-fourier-transform/