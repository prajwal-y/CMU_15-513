/* 
 * CS:APP Data Lab 
 * 
 * Prajwal Yadapadithaya (pyadapad)
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int evenBits(void) {
  /*Hard code the value with all even bits set*/
  return (0x55<<24)|(0x55<<16)|(0x55<<8)|(0x55);
}
/* 
 * isEqual - return 1 if x == y, and 0 otherwise 
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
  /*Simple XOR does the trick*/
  return !(x^y);
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
   /* Right shift and left shift the bytes to be swapped by n*8/m*8 and m*8/n*8 times respectively and or with the remaining bytes of x */
    int s1 = n << 3;
    int s2 = m << 3;
    int temp1 = 0xFF << s1;
    int temp2 = 0xFF << s2;
    return ((((temp1 & x) >> s1) << s2) & temp2) | ((((temp2 & x) >> s2) << s1) & temp1) | ((~temp1 & ~temp2) & x);
}
/* 
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateRight(0x87654321,4) = 0x18765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateRight(int x, int n) {
  /*Store the n rightmost bits temporarily and or it with x and right shifting it by n bits*/
  int leftShift = 32 + (~n + 1);
  int temp = ((~(1 << 31) >> n) << 1 ) | 1;
  return (x << leftShift) | ((x >> n) & temp);
}
/* 
 * logicalNeg - implement the ! operator using any of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  /* Firstly, convert all x to positive, and then subtract 1. Only in the case of 0, the sign bit will be set.*/
  int check = 1 << 31;
  return (((((x & ~check) + ~0) & check) >> 31) & 1) & ((x >> 31) + 1);
}
/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  /*Simple hardcoding*/
  return ~(1 << 31);
}
/* 
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 2
 */
int sign(int x) {
    /* Check for sign bit and handle the case for 0 separately*/
    return !(!x) | ((x & (1 << 31)) >> 31);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  /* Compute x-y and check for sign bit and handle the case for positive and negative overflow*/
  int check = 1 << 31;
  int first = !(x & check);
  int second = !(y & check);
  int basicCheck = !((x + (~y + 1) + ~0) & check);
  return (basicCheck & !(!first & second)) | (first & !second);
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
  /* Similar to the previous quesion, the only differnce being the final condition for return*/
  int check = 1 << 31;
  int first = !(x & check);
  int second = !(y & check);
  int basicCheck = !((x + (~y + 1)) & check);
  return !((basicCheck & (!first & second)) | (first & !second & !basicCheck));
}
/*
 * satAdd - adds two numbers but when positive overflow occurs, returns
 *          maximum possible value, and when negative overflow occurs,
 *          it returns minimum possible value.
 *   Examples: satAdd(0x40000000,0x40000000) = 0x7fffffff
 *             satAdd(0x80000000,0xffffffff) = 0x80000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 30
 *   Rating: 4
 */
int satAdd(int x, int y) {
  /* Add x and y, handling the cases of overflows separately*/
  int check = 1 << 31;
  int sum = x + y;
  int first = !(x & check);
  int second = !(y & check);
  int basicCheck = !(sum & check);
  int pOverflow = basicCheck | !first | !second;
  int nOverflow = !first & !second & basicCheck;
  return (~check & (~0 + pOverflow)) | (nOverflow << 31) | (sum & (~0 + (!pOverflow | nOverflow)));
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  /*Count the number of bits by right shifting 16,8,4,2,1 times in order, each time keping track of the remaining bits. And sum the count to get the final tally.*/
  int test = ~0;
  int sign = !((1<<31) & x);
  int count = 1;
  int tmp = 0;
  x = (((test + sign) & (~x + 1)) | ((test + !sign) & x)) & ~(1 << 31);
  count = count & (!(!(x & (x + test))) | (sign & !(!x)));
  count = count + (!x) + ((test +! (!x & !sign)) & 31);
  tmp = (test + !(x >> 16)) & 16;
  x >>= tmp; count += tmp; tmp = 0;
  
  tmp = (test + !(x >> 8)) & 8;
  x >>= tmp; count += tmp; tmp = 0;

  tmp = (test + !(x >> 4)) & 4;
  x >>= tmp; count += tmp; tmp = 0;
  
  tmp = (test + !(x >> 2)) & 2;
  x >>= tmp; count += tmp; tmp = 0;
  
  tmp = (test + !(x >> 1)) & 1;
  x >>= tmp; count += tmp; tmp = 0; 
  
  count = count + (x & 1);
  return count;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  /* Represent 0.5 in single precision floating point representation and multiply the two numbers by adding the exponent and multiplying the fraction part*/
  int e, m, fs, fe, check, temp;
  check = 1 << 31;
  fs = uf & check;
  e = (uf >> 23) & 0xFF;
  fe = e;
  if(e == 255)
	return uf;
  if(e)
    temp = e - 128;
  else
    temp = -127;
  m = uf & 0x7FFFFF;
  if(temp == -127) {
   int n1 = ((fe << 23) | m);
   int m1 = n1 >> 1;
   return (fs | m1) + (n1 & m1 & 1);
  }
  else
    fe = ((127 + temp) << 23) & ~check;
  return fs | fe | m; 
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  /* Right shift if exponent-23 is greater than 0, left shift if less than 0. Handle the cases for NaN and infinity separately*/
  int E, exp, m, s, e;
  int check = 1 << 31;
  s = uf & check;
  e = (uf >> 23) & 0xFF;
  m = uf & 0x7FFFFF;
  if(e == 255)
    return check;

  if(e)
    E = e - 127;
  else
    E = -126;

  if(E > 30)
    return check;
  else if(E < 0)
    return 0;
  else if(E > 22)
    exp = m << (E - 23);
  else
    exp = m >> (23 - E);

  exp = exp + (1 << E);    
   
  if(s)
    exp = (~exp + 1);
    
  return exp;

}
