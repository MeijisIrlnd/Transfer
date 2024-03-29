# Transfer
![CMake](https://github.com/MeijisIrlnd/Transfer/actions/workflows/ci.yml/badge.svg)
#### because normal distortions are boring
![image](https://user-images.githubusercontent.com/38221014/210483486-0d4fb6f0-b8e3-4efa-83a4-afd8b2d7dabe.png)
## about 
Transfer is a waveshaper based distortion, except the transfer function is JIT compiled from text inputted by the user. Starting as a tool for testing different transfer functions in other projects, it has evolved into a pretty cool and unique plugin (<i>disclaimer it might not be that unique I have done absolutely no market research whatsoever)</i>. It also has a built in gate, and an emphasis filter, which applies the filters to the signal pre waveshaping, then applies the exact opposite filter post waveshaping. Due to popular demand (by popular I mean that one guy on reddit), it now also has has <b>16x</b> <b><i><u>oversampling</b></i></u> and you can't turn it off sorry I don't make the rules.

## what that means (how to use)
there's a list of functions / operators which is pretty hard to miss, and a text field after the `H(s) = ` where you can type an expression in, and it will compile on return key press. In terms of variables, you have access to the input sample (<b>x</b>), the value from the distortion coefficient slider (<b>d</b>) (<i>note: its range is 0 to 10, fairly arbitrarily</i>), the value from the y slider (<b>y</b>), and the value from the z slider (<b>z</b>), which I'd recommend using as a volume control (more on in a sec..). There are also 4 general purpose registers (<b>gpr[n], where n is the register you want to access</b>), which can be used to store abritrary values. The registers get cleared on expression change, and there's also a <b>CLR GPR</b> button below the filter panel you can use to zero them. The expression input supports conditionals; <br>`if(expr1 evaluates to true) expr2; else expr3;`, and you can evaluate multiple expressions (from left to right) with the syntax `~(expr1, expr2, ... exprn)`, where the last value of the expression is what gets returned. This means you can have different transfer functions depending on the whether the sample is increasing or decreasing when combined with the registers, if that's something you're into. Assignment is possible but a little weird syntactically, `:=` is regular assignment. You can access the gate and filter panels with the respective gate and filter buttons, and get back to the graph view with, you guessed it, the graph button. 

## health and safety
As is the nature of something like this, it's pretty easy to annihilate your headphones, speakers, eardrums, nearby wine glasses and small woodland creatures. If you're messing around with this and aren't sure what you're doing (like me), I'd actually recommend turning down the volume on whatever is driving this to 0 before trying a function, and slowly turning it up, to be sure it's not going to cause you physical or mental harm. if that falls on <i>deaf ears</i> (hint) <b>please at least put your entire expression in parentheses, multiply by z, and turn the z slider all the way down before you hit enter.</b> This paragraph is now longer than the instructions leading me to believe I have created a novel new WMD, but in all seriousness, be careful especially with logs, or anything that approaches infinity or anything wild in that vein. There's a <b>panic</b> button below the graph, which will revert your transfer function to `H(s) = x`, which will just pass whatever input it has through the plugin, useful to avoid a visit to a&e or a noise complaint.  

## crashes 
as far as i can tell, most of the obvious crashes are sorted (checking for nan / inf, division by zero, all the boring stuff), but if you find anything spicy that crashes it do let me know. if you type in an expression which can't compile, it'll just revert to linear (`H(s) = x`), and if it finds nans or infs or -nans or -infs, it zeroes them.

## roadmap 
- <s>resizable</s>
- syntax highlighting of some way shape or form, starting with brace pairs
- ???

## starting points
sigmoid functions tend to be a nice starting point, [the wikipedia article for them](https://en.wikipedia.org/wiki/Sigmoid_function) has a bunch of examples. some functions i've been using to test, though, are:
- `(tanh(x * d^3) / tanh(d^3)) * z` is a nice starting point, the exponent makes it 'clippier', and dividing by the numerator without the x term means the result can't go above 1 
- `(sinc(x * d) / (sinc(d)) * z` does a [Kahn & Neek - Percy](https://www.youtube.com/watch?v=R4XlhUBOXV8) style distortion on kicks, pretty wacky
- `(atan(x * d) / atan(d)) * z` seems to be a harsher version of tanh
- `~(if(gpr[1] >= z * 100) ~(gpr[0] := x, gpr[1] := 0);, gpr[1] += 1, gpr[0])` is a downsampling style effect making use of a sample and hold algorithm, moving the z slider in this case changes the duration of the hold 
## licensing, etc: 
transfer makes use of the following: 
- [juce](https://juce.com/)
- [exprtk](http://www.partow.net/programming/exprtk/)
- [SDSP (my fledgling DSP helper library)](https://github.com/MeijisIrlnd/SDSP)
- [asio (low latency in standalone...)](https://www.asio4all.org/)
- android's droid sans mono font for that calculator aesthetic 
- also tried [atmsp](https://github.com/ArashPartow/math-parser-benchmark-project/blob/master/atmsp/atmsp.h) and ![mathpresso](https://github.com/kobalicek/mathpresso) before settling on exprtk, but some files from those may still be in the project or on other branches




