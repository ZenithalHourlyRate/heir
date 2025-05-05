module attributes {backend.lattigo, bgv.schemeParam = #bgv.scheme_param<logN = 14, Q = [35184372744193, 35184373006337, 35184373989377, 35184376545281, 35184377331713, 35184378511361, 35184378707969, 35184379035649], P = [35184380870657, 35184381591553, 35184381853697], plaintextModulus = 65537>, scheme.bgv} {
  func.func @mult_indep(
      %arg0: i16 {secret.secret},
      %arg1: i16 {secret.secret},
      %arg2: i16 {secret.secret},
      %arg3: i16 {secret.secret},
      %arg4: i16 {secret.secret},
      %arg5: i16 {secret.secret},
      %arg6: i16 {secret.secret},
      %arg7: i16 {secret.secret},
      %arg8: i16 {secret.secret},
      %arg9: i16 {secret.secret},
      %arg10: i16 {secret.secret},
      %arg11: i16 {secret.secret}
      ) -> i16 {
      %0 = arith.muli %arg0, %arg1 : i16
      %1 = arith.muli %0, %arg2 : i16
      %2 = arith.muli %1, %arg3 : i16
      %3 = arith.muli %2, %arg4 : i16
      %4 = arith.muli %3, %arg5 : i16
      %5 = arith.muli %4, %arg6 : i16
      %6 = arith.muli %5, %arg7 : i16
      %7 = arith.muli %6, %arg8 : i16
      %8 = arith.muli %7, %arg9 : i16
      %9 = arith.muli %8, %arg10 : i16
      %10 = arith.muli %9, %arg11 : i16
      return %10 : i16
  }
}
