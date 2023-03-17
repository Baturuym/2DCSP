\ENCODING=ISO-8859-1
\Problem name: IloCplex

Minimize
 obj1: Y_1 + Y_2 + x3 + x4 + x5 + x6 + x7 + x8
Subject To
 c1:  2 X_1 + X_3 >= 3
 c2:  X_3 + 3 X_2 >= 4
 c3:  4 X_4 >= 4
 c4:  2 X_3 + X_4 >= 3
 c5:  3 X_1 + 3 X_2 >= 6
 c6:  Y_1 + Y_2 + x5 + x7 + x8 - X_1 - X_3 >= 0
 c7:  >= 0
 c8:  >= 0
 c9:  Y_2 + 5 x3 + 5 x4 + 2 x5 + 5 x6 + 2 x7 + 2 x8 - X_4 >= 0
 c10: Y_1 - X_2 >= 0
End
