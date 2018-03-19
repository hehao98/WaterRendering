# 真实感海洋的绘制（二）：使用快速傅里叶变换加速波形计算
其实上一篇博文所写的$H(\vec{x},t)​$，就是二维傅里叶变换的求和式，之前的暴力计算法属于二维的离散傅里叶变换（Discrete Fourier Transform, DFT），利用二维的快速傅里叶变换（Fast Fourier Transform, FFT）可以将复杂度从$O(n^4)​$降低到$O(n^2\log{n})​$。

如果读者不熟悉FFT，强烈建议阅读《算法导论》相关章节，个人感觉没有什么资料讲得比《算法导论》更清楚了。书后习题还有关于二维FFT的思考，是本文算法正确性的理论基础。

## 数学推导

### 高度场

出于数学上推导的简单和实现上的简单，我们约定$L_x=L_y=N=M=2^k$，把方程重写为如下形式
$$
H(\vec x,t) = \sum_{\vec{k}}h(\vec k, t)e^{i\vec{k}\cdot \vec{x}} \mbox{,  where }
\vec{k}=(2\pi n/L,2\pi m/L), \ \vec{x}=(x, z)\\
-N/2\le n, m < N/2
$$

将$\vec{k}$与$\vec{x}$带入整理得
$$
\begin{align}
H(\vec{x},t)&=\sum_{n=-N/2}^{N/2 - 1}\sum_{m=-N/2}^{N/2-1}h(\vec{k},t)e^{2\pi nxi/N+2\pi mz/N} \\
&=\sum_{n=-N/2}^{N/2-1}e^{2\pi nxi/N}\sum_{m=-N/2}^{N/2-1}h(\vec{k},t)e^{2\pi mzi/N}
\end{align}
$$

为了化成便于使用FFT的形式，令$n'=n+N/2$, $m'=m+N/2$
$$
H(\vec{x},t)=\sum_{n'=0}^{N-1}e^{2\pi(n'-N/2)xi/N}\sum_{m'=0}^{N-1}h(\vec{k},t)e^{2\pi(m'-N/2)zi/N}
$$
其中
$$
\begin{align}
e^{2\pi(n'-N/2)xi/N}&=e^{2\pi n'xi/N}e^{-\pi xi}\\
&=(-1)^x e^{2\pi n'xi/N} (\mbox{ note that }e^{\pi i}=-1, N\ge 2)
\end{align}
$$
所以
$$
H(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}h(\vec{k},t)e^{2\pi m'zi/N}
$$
这样已经成为了可以进行FFT的形式。这儿出现了一个$(-1)^{x+z}$，不用担心，在全部计算完之后带入即可。

### 法线

关于法线的计算有一些tricky。我们先写出之前给出的法线公式
$$
\epsilon(\vec{x},t)=\nabla H(\vec{x},t)=\sum_{\vec{k}}i\vec{k}h(\vec{k},t)e^{i\vec{k}\cdot \vec{x}}\\
\begin{align}
\vec{N}(\vec{x},t)&=(0,1,0)-(\epsilon_x(\vec{x},t),0,\epsilon_z(\vec{x},t))\\
&=(-\epsilon_x(\vec{x},t),1,-\epsilon_z(\vec{x},t))
\end{align}
$$
问题在于计算$\epsilon(\vec{x},t) = (\epsilon_x, \epsilon_z)$。我们首先类似$H$的推导得到
$$
\epsilon(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}i\vec{k}h(\vec{k},t)e^{2\pi m'zi/N}
$$
经过观察我们发现$\epsilon(\vec{x},t)$的两个方向分别只与$\vec{k}$的两个方向有关，那么可以写成如下形式
$$
\epsilon_x(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}ik_xh(\vec{k},t)e^{2\pi m'zi/N}\\
\epsilon_z(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}ik_zh(\vec{k},t)e^{2\pi m'zi/N}\\
$$
然后分别计算即可。

### 偏置量

类似地，我们写出偏置量的公式
$$
\vec{D}(\vec{x},t)=\sum_{\vec{k}}-i\frac{\vec{k}}{|\vec{k}|}
h(\vec{k},t)e^{i\vec{k}\cdot \vec{x}}
$$
和法线计算的方式完全一样得到
$$
\vec{D}(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}-i\frac{\vec{k}}{|\vec{k}|}h(\vec{k},t)e^{2\pi m'zi/N}
$$
分成两个方向
$$
D_x(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}-i\frac{k_x}{|\vec{k}|}h(\vec{k},t)e^{2\pi m'zi/N} \\
D_z(\vec{x},t)=(-1)^{x+z}\sum_{n'=0}^{N-1}e^{2\pi n'xi/N}\sum_{m'=0}^{N-1}-i\frac{k_z}{|\vec{k}|}h(\vec{k},t)e^{2\pi m'zi/N}
$$
这就完成了推导的工作。相信熟悉FFT的读者可以对上边的公式很容易给出一个自己的FFT计算实现。

## 实现结果

![FFT](https://images2018.cnblogs.com/blog/1322089/201803/1322089-20180319192142940-2103236491.gif)

之后的博客将会研究如何对这样的海面进行真实感渲染和光照计算。如果有时间的话，可能会探索如何把这个FFT模型移植到GPU上并行计算以实现更好的性能。

## 参考文献

1. Tessendorf, Jerry. Simulating Ocean Water. *In SIGGRAPH 2002 Course Notes #9 (Simulating Nature: Realistic and Interactive Techniques)*, ACM Press.
2. Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest and Clifford Stein. *Introduction to Algorithms, 3rd Edition*, MIT Press.