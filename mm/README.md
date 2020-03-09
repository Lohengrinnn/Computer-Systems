enhanced K&R memory allocator
self implementation of memory allocator using doubly linked list

K&R implementation:
Results for traces:
index  leaks errors     ops      secs    Kops  file
    1      0      0      12  0.000042     285  short1-bal.rep
    2      0      0      12  0.000027     444  short2-bal.rep
    3      0      0    5694  0.006146     926  traces/trace0.rep
    4      0      0    5848  0.005170    1131  traces/trace1.rep
    5      0      0    6648  0.007322     907  traces/trace2.rep
    6      0      0    5380  0.005356    1004  traces/trace3.rep
    7      0      0   14400  0.010173    1415  traces/trace4.rep
    8      0      0    4800  0.014188     338  traces/trace5.rep
    9      0      0    4800  0.012743     376  traces/trace6.rep
   10      0      0   12000  0.048118     249  traces/trace7.rep
   11      0      0   24000  0.091155     263  traces/trace8.rep
   12      0      0   14401  0.127076     113  traces/trace9.rep
   13      0      0   14401  0.013204    1090  traces/trace10.rep

Doubly linked list Implementation:
Results for traces:
index  leaks errors     ops      secs    Kops  file
    1      0      0      12  0.000048     249  short1-bal.rep
    2      0      0      12  0.000022     545  short2-bal.rep
    3      0      0    5694  0.006544     870  traces/trace0.rep
    4      0      0    5848  0.004623    1264  traces/trace1.rep
    5      0      0    6648  0.006397    1039  traces/trace2.rep
    6      0      0    5380  0.004835    1112  traces/trace3.rep
    7      0      0   14400  0.010786    1335  traces/trace4.rep
    8      0      0    4800  0.008477     566  traces/trace5.rep
    9      0      0    4800  0.006624     724  traces/trace6.rep
   10      0      0   12000  0.017336     692  traces/trace7.rep
   11      0      0   24000  0.029245     820  traces/trace8.rep
   12      0      0   14401  0.137807     104  traces/trace9.rep
   13      0      0   14401  0.014237    1011  traces/trace10.rep


frame code from Philip Gust, Professor of Northeastern University
