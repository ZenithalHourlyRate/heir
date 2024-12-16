!Z1095233372161_i64_ = !mod_arith.int<1095233372161 : i64>
#inverse_canonical_embedding_encoding = #lwe.inverse_canonical_embedding_encoding<cleartext_start = 16, cleartext_bitwidth = 16>
!rns_L0_ = !rns.rns<!Z1095233372161_i64_>
#ring_rns_L0_1_x4_ = #polynomial.ring<coefficientType = !rns_L0_, polynomialModulus = <1 + x**4>>
!rlwe_pt_L0_ = !lwe.rlwe_plaintext<encoding = #inverse_canonical_embedding_encoding, ring = #ring_rns_L0_1_x4_, underlying_type = tensor<4xi16>>
#rlwe_params_L0_ = #lwe.rlwe_params<ring = #ring_rns_L0_1_x4_>
!rlwe_ct_L0_ = !lwe.rlwe_ciphertext<encoding = #inverse_canonical_embedding_encoding, rlwe_params = #rlwe_params_L0_, underlying_type = tensor<4xi16>>
module {
  func.func @func(%arg0: !openfhe.crypto_context, %arg1: !rlwe_ct_L0_) -> !rlwe_ct_L0_ {
    %0 = openfhe.bootstrap %arg0, %arg1 : (!openfhe.crypto_context, !rlwe_ct_L0_) -> !rlwe_ct_L0_
    return %0 : !rlwe_ct_L0_
  }
}
