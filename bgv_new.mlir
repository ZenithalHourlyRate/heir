// #ring = #polynomial.ring<coefficientType = i32, coefficientModulus = 463187969 : i32, polynomialModulus = <1 + x**8>>
// #rlwe_params = #lwe.rlwe_params<ring = #ring>
// #rlwe_params_3 = #lwe.rlwe_params<dimension = 3, ring = #ring>
// !plaintext = !lwe.rlwe_plaintext<encoding = #encoding, ring = #ring, underlying_type = tensor<8xi16>>
// !ciphertext = !lwe.rlwe_ciphertext<encoding = #encoding, rlwe_params = #rlwe_params, underlying_type = tensor<8xi16>>
// !ciphertext2 = !lwe.rlwe_ciphertext<encoding = #encoding, rlwe_params = #rlwe_params_3, underlying_type = tensor<8xi16>>
// !ciphertext_small = !lwe.rlwe_ciphertext<encoding = #encoding, rlwe_params = #rlwe_params, underlying_type = i16>

#generator = #polynomial.int_polynomial<1 + x**8192>
#plain_ring = #polynomial.ring<coefficientType = i64, coefficientModulus = 65537 : i64, polynomialModulus=#generator>
#cipher_ring = #polynomial.ring<coefficientType = i64, coefficientModulus = 60093809372265 : i64, polynomialModulus=#generator>

// Application Data Information
#preserve_overflow = #lwe.preserve_overflow<>
#application_data = #lwe.application_data<message_type = i16, overflow = #preserve_overflow>

// Plaintext Space Information
#inverse_canonical_enc = #lwe.inverse_canonical_encoding<scaling_factor = 10000>
#plaintext_space = #lwe.plaintext_space<ring = #plain_ring, encoding = #inverse_canonical_enc>

// Ciphertext Space Information
#ciphertext_space = #lwe.ciphertext_space<ring = #cipher_ring, encryption_type = lsb>

// Modulus Chain info (for RLWE)
#modulus_chain = #lwe.modulus_chain<elements = <35156991246337, 35175245135873, 35155917488129>, current = 0>

// Key Information
#key = #lwe.key<id = "1234", size = 1>

// New Types
!secret_key = !lwe.new_lwe_secret_key<key = #key, ring = #cipher_ring>
!public_key = !lwe.new_lwe_public_key<key = #key, ring = #cipher_ring>
!new_lwe_plaintext = !lwe.new_lwe_plaintext<application_data = #application_data, plaintext_space = #plaintext_space>
!new_lwe_ciphertext = !lwe.new_lwe_ciphertext<application_data = #application_data, plaintext_space = #plaintext_space, key = #key, ciphertext_space = #ciphertext_space, modulus_chain = #modulus_chain>

!plaintext = !new_lwe_plaintext
!ciphertext = !new_lwe_ciphertext

module {
  func.func @dot_product(%p: !plaintext) -> !ciphertext {
    //%c = bgv.encrypt %p : !plaintext -> !ciphertext
    //%0 = bgv.add %c, %c : !ciphertext
    //%1 = bgv.my_mul %c, %c : (!ciphertext, !ciphertext) -> !ciphertext

    %arg0 = bgv.encrypt %p : !plaintext -> !ciphertext
    %arg1 = bgv.encrypt %p : !plaintext -> !ciphertext
    %0 = bgv.my_mul %arg0, %arg1 : (!ciphertext, !ciphertext) -> !ciphertext
    %1 = bgv.my_relinearize %0 : !ciphertext -> !ciphertext
    %2 = bgv.my_rotate %1 {offset = 4 : index} : !ciphertext
    %3 = bgv.add %1, %2 : !ciphertext
    %4 = bgv.my_rotate %3 {offset = 2 : index} : !ciphertext
    %5 = bgv.add %3, %4 : !ciphertext
    %6 = bgv.my_rotate %5 {offset = 1 : index} : !ciphertext
    %7 = bgv.add %5, %6 : !ciphertext
    return %0 : !ciphertext
  }
}
