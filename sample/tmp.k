fungsi fibi(x)
  var a = 1, b = 1, c=2 in
  (for i = 3; i < x; 1; then
     c = a + b :
     a = b :
     b = c) :
  b;

fibi(10);
