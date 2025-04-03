module attributes {backend.lattigo, bgv.schemeParam = #bgv.scheme_param<logN = 13, Q = [35184372744193, 35184373006337, 35184373989377, 35184376545281, 35184377331713, 35184378511361, 35184378707969, 35184379035649], P = [35184380870657, 35184381591553, 35184381853697], plaintextModulus = 65537>, scheme.bgv} {
  func.func @mult_dep(%0: i16 {secret.secret}) -> i16 {
    %1 = arith.muli %0, %0 : i16
    %2 = arith.muli %1, %0 : i16
    %3 = arith.muli %2, %0 : i16
    %4 = arith.muli %3, %0 : i16
    %5 = arith.muli %4, %0 : i16
    %6 = arith.muli %5, %0 : i16
    %7 = arith.muli %6, %0 : i16
    return %7 : i16
  }
}
