Raul Escobar
raulescobar_g@tamu.edu
328003859

[x] Completed N-link inverse kinematics

#### Gradient Descent with Line Search and Newtons Method with Line Search

Change number of links on line 37 in main.cpp

If number of links is changed to < 10 also change the alpha to 1e0 for faster convergence as outlined in the instructions from professor

if gradient descent less than tolerance, then it returns early (does not compute Newtons Method) to keep smooth fps
