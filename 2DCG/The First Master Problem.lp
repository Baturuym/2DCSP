\ENCODING=ISO-8859-1
\Problem name: IloCplex

Minimize
 obj1: Y_1 + Y_2
Subject To
 c1:  Y_1 + Y_2 - X_1 - X_3 >= 0
 c2:  >= 0
 c3:  >= 0
 c4:  Y_2 - X_4 >= 0
 c5:  Y_1 - X_2 >= 0
 c6:  2 X_1 + X_3 >= 3
 c7:  X_3 + 3 X_2 >= 4
 c8:  4 X_4 >= 4
 c9:  2 X_3 + X_4 >= 3
 c10: 3 X_1 + 3 X_2 >= 6
Bounds
      Y_1 >= 0
      Y_2 >= 0
      X_1 >= 0
      X_3 >= 0
      X_4 >= 0
      X_2 >= 0
Generals
 Y_1  Y_2  X_1  X_3  X_4  X_2 
End
