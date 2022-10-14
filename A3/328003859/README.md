Raul Escobar
328003859
raulescobar_g@tamu.edu

Did all the normal tasks (not bonus)

Incompatible blendshapes: 
    
    It would make sense that two blend shapes that modify the same part of a mesh would have undesirable behavior. 
    I think this is because the blendshapes are additive, so for example, if we have two blend shapes that have a similar   
    deltas for the position, they would create exaggerated expressions similar to having the weights go above 1.0

For the incompatible blend shapes uncomment line 13 and comment line 10 in the input.txt



For the FACS, uncomment either line 16 or 19, and comment out 10 and 13