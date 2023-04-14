\ENCODING=ISO-8859-1
\Problem name: IloCplex

Minimize
 obj1: Y_1 + Y_2 + Y_3 + Y_4 + Y_5 + Y_6 + Y_7
Subject To
 c1:  Y_1 + Y_2 + Y_4 + Y_7 - X_1 - X_3 >= 0
 c2:  >= 0
 c3:  >= 0
 c4:  Y_2 + 5 Y_3 + 2 Y_4 + Y_6 - X_4 >= 0
 c5:  Y_1 + 3 Y_5 + 3 Y_6 + Y_7 - X_2 >= 0
 c6:  2 X_1 + X_3 >= 3
 c7:  X_3 + 3 X_2 >= 4
 c8:  4 X_4 >= 4
 c9:  2 X_3 + X_4 >= 3
 c10: 3 X_1 + 3 X_2 >= 6
Bounds
      Y_1 = 1
End
