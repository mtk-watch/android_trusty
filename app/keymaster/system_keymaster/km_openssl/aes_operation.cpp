/*
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "aes_operation.h"

namespace keymaster {

static const keymaster_block_mode_t supported_block_modes[] = {KM_MODE_ECB, KM_MODE_CBC,
                                                               KM_MODE_CTR, KM_MODE_GCM};

const keymaster_block_mode_t*
AesEvpCipherDescription::SupportedBlockModes(size_t* block_mode_count) const {
    *block_mode_count = array_length(supported_block_modes);
    return supported_block_modes;
}

const EVP_CIPHER* AesEvpCipherDescription::GetCipherInstance(size_t key_size,
                                                             keymaster_block_mode_t block_mode,
                                                             keymaster_error_t* error) const {
    *error = KM_ERROR_OK;

    switch (block_mode) {
    case KM_MODE_ECB:
        switch (key_size) {
        case 16:
            return EVP_aes_128_ecb();
        case 24:
            return EVP_aes_192_ecb();
        case 32:
            return EVP_aes_256_ecb();
        };
        *error = KM_ERROR_UNSUPPORTED_KEY_SIZE;
        break;

    case KM_MODE_CBC:
        switch (key_size) {
        case 16:
            return EVP_aes_128_cbc();
        case 24:
            return EVP_aes_192_cbc();
        case 32:
            return EVP_aes_256_cbc();
        };
        *error = KM_ERROR_UNSUPPORTED_KEY_SIZE;
        break;

    case KM_MODE_CTR:
        switch (key_size) {
        case 16:
            return EVP_aes_128_ctr();
        case 24:
            return EVP_aes_192_ctr();
        case 32:
            return EVP_aes_256_ctr();
        }
        *error = KM_ERROR_UNSUPPORTED_KEY_SIZE;
        break;

    case KM_MODE_GCM:
        switch (key_size) {
        case 16:
            return EVP_aes_128_gcm();
        case 24:
            return EVP_aes_192_gcm();
        case 32:
            return EVP_aes_256_gcm();
        }
        *error = KM_ERROR_UNSUPPORTED_KEY_SIZE;
        break;

    default:
        *error = KM_ERROR_UNSUPPORTED_BLOCK_MODE;
        break;
    }

    assert(*error != KM_ERROR_OK);
    return nullptr;
}

static AesEvpCipherDescription description;
const EvpCipherDescription& AesOperationFactory::GetCipherDescription() const {
    return description;
}

}  // namespace keymaster
