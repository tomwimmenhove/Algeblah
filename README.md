# Algeblah
Messing around with an algoritm that re-arranges equations

There's an online demo available: https://algeblah.co.uk/

Here's an example of it's usage:
```
$ algeblah 
Algebla: An equation solving, arbitrary precision calculator
Copyright (C) 2022 Tom Wimmenhove

This is free software with ABSOLUTELY NO WARRANTY.
For details type `warranty'. 

> p1 = 1; v1 = 2; t1 = 3; p2 = 4; v2 = 5
  p1 = 1
  v1 = 2
  t1 = 3
  p2 = 4
  v2 = 5
> solve t2: p1 * v2 / t1 = p2 * v2 / t2
  p2 * v2 / (p1 * v2 / t1) = 12
> t2
  t2 = 12
> 
> b = 4; c = 5
  b = 4
  c = 5
> solve a: a^2 + b^2 = c^2
  sqrt(c ^ 2 - b ^ 2) = 3
> 
> p2 = 100
  p2 = 100
> db = 20
  db = 20
> solve p1: db = 10 * log(p1 / p2) / log(10)
  %e ^ (db * log(10) / 10) * p2 = 10000
> 
```
