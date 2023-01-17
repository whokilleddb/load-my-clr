# Load My CLR

![](./img/poc.png)

Run managed code from unmanaged process!

This repository comtains Proof-of-Concept code to launch managed code from an unmanaged process and is directly inspired from [Adam Chester's Blog](https://www.mdsec.co.uk/2020/03/hiding-your-net-etw/).

## Compilation
First, compile the C# code with: 
```
csc /target:library /out:helloworld.dll helloworld.cs
```

Remember to update the Path to the resulting executable in `customloader.h` and compile it with:
```
cl customloader.c
```
