# 真实感海洋的绘制（二）：使用快速傅里叶变换加速波形计算
其实上一篇博文所写的$H(\vec{x},t)$，就是二维傅里叶变换的求和式，之前的暴力计算法属于二维的离散傅里叶变换（DFT），二维的快速傅里叶变换（FFT）可以将复杂度从$O(n^4)$降低到$O(n^2\log{n})$。

如果读者不熟悉FFT，强烈建议阅读《算法导论》相关章节，个人感觉没有什么资料讲得比《算法导论》更清楚了。书后习题还有关于二维FFT的结论，是本文算法正确性的理论基础。

## 数学推导

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
这儿出现了一个$(-1)^{x+z}$，不用担心，在计算完之后再乘即可。

我们着重考虑其右半部分，因为只要对$m$求和成功那也能用相同的方法对$n$求和。

我们将其拆分成$m$为奇数的和式和$m$为偶数的和式
$$
\begin{align}
\sum_{m'=0}^{N-1}h(\vec{k},t)e^{2\pi m'zi/N}=\sum_{m'=0}^{N/2-1}h(\vec{k},t)e^{2\pi zi(2m')/N}+e^{2\pi zi/N}\sum_{m'=0}^{N/2-1}h(\vec{k}, t)e^{2\pi zi(2m'+1)/N}
\end{align}
$$
