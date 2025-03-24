module attributes {bgv.schemeParam = #bgv.scheme_param<logN = 15, Q = [1152921504608747521, 1152921504614055937, 1152921504615628801, 1152921504615694337, 1152921504616480769, 1152921504616808449, 1152921504618381313, 1152921504620347393], P = [1152921504621199361, 1152921504621985793, 1152921504622510081], plaintextModulus = 65537>, scheme.bgv} {
  func.func @mult_dep(
      %arg0: i16 {secret.secret}
      ) -> i16 {
      %0 = arith.muli %arg0, %arg0 : i16
      %1 = arith.muli %0, %arg0 : i16
      %2 = arith.muli %1, %arg0 : i16
      %3 = arith.muli %2, %arg0 : i16
      %4 = arith.muli %3, %arg0 : i16
      %5 = arith.muli %4, %arg0 : i16
      %6 = arith.muli %5, %arg0 : i16
      return %6 : i16
  }
}
