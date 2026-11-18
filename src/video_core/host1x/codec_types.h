// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>
#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"

namespace Tegra {

enum class Vp9SurfaceIndex : u32 {
    Last = 0,
    Golden = 1,
    AltRef = 2,
    Current = 3,
};

struct Vp9FrameDimensions {
    s16 width;
    s16 height;
    s16 luma_pitch;
    s16 chroma_pitch;
};
static_assert(sizeof(Vp9FrameDimensions) == 0x8, "Vp9 Vp9FrameDimensions is an invalid size");

enum class FrameFlags : u32 {
    IsKeyFrame = 1 << 0,
    LastFrameIsKeyFrame = 1 << 1,
    FrameSizeChanged = 1 << 2,
    ErrorResilientMode = 1 << 3,
    LastShowFrame = 1 << 4,
    IntraOnly = 1 << 5,
};
DECLARE_ENUM_FLAG_OPERATORS(FrameFlags)

enum class TxSize {
    Tx4x4 = 0,
    Tx8x8 = 1,
    Tx16x16 = 2,
    Tx32x32 = 3,
    TxSizes = 4
};

enum class TxMode {
    Only4X4 = 0,
    Allow8X8 = 1,
    Allow16X16 = 2,
    Allow32X32 = 3,
    TxModeSelect = 4,
    TxModes = 5
};

struct Segmentation {
    u8 enabled;
    u8 update_map;
    u8 temporal_update;
    u8 abs_delta;
    std::array<u32, 8> feature_mask;
    std::array<std::array<s16, 4>, 8> feature_data;
};
static_assert(sizeof(Segmentation) == 0x64, "Segmentation is an invalid size");

struct LoopFilter {
    u8 mode_ref_delta_enabled;
    std::array<s8, 4> ref_deltas;
    std::array<s8, 2> mode_deltas;
};
static_assert(sizeof(LoopFilter) == 0x7, "LoopFilter is an invalid size");

// Full EntropyProbs and additional structure definitions can be referenced in source [11].
struct Vp9EntropyProbs {
    std::array<u8, 36> y_mode_prob;
    std::array<u8, 64> partition_prob;
    std::array<u8, 1728> coef_probs;
    std::array<u8, 8> switchable_interp_prob;
    std::array<u8, 28> inter_mode_prob;
    std::array<u8, 4> intra_inter_prob;
    std::array<u8, 5> comp_inter_prob;
    std::array<u8, 10> single_ref_prob;
    std::array<u8, 5> comp_ref_prob;
    std::array<u8, 6> tx_32x32_prob;
    std::array<u8, 4> tx_16x16_prob;
    std::array<u8, 2> tx_8x8_prob;
    std::array<u8, 3> skip_probs;
    std::array<u8, 3> joints;
    std::array<u8, 2> sign;
    std::array<u8, 20> classes;
    std::array<u8, 2> class_0;
    std::array<u8, 20> prob_bits;
    std::array<u8, 12> class_0_fr;
    std::array<u8, 6> fr;
    std::array<u8, 2> class_0_hp;
    std::array<u8, 2> high_precision;
};
static_assert(sizeof(Vp9EntropyProbs) == 0x7B4, "Vp9EntropyProbs is an invalid size");

struct Vp9PictureInfo {
    u32 bitstream_size;
    std::array<u64, 4> frame_offsets;
    std::array<s8, 4> ref_frame_sign_bias;
    s32 base_q_index;
    s32 y_dc_delta_q;
    s32 uv_dc_delta_q;
    s32 uv_ac_delta_q;
    s32 transform_mode;
    s32 interp_filter;
    s32 reference_mode;
    s32 log2_tile_cols;
    s32 log2_tile_rows;
    std::array<s8, 4> ref_deltas;
    std::array<s8, 2> mode_deltas;
    Vp9EntropyProbs entropy;
    Vp9FrameDimensions frame_size;
    u8 first_level;
    u8 sharpness_level;
    bool is_key_frame;
    bool intra_only;
    bool last_frame_was_key;
    bool error_resilient_mode;
    bool last_frame_shown;
    bool show_frame;
    bool lossless;
    bool allow_high_precision_mv;
    bool segment_enabled;
    bool mode_ref_delta_enabled;

    Segmentation segmentation;
    std::array<u8, 7> seg_tree_probs;
    std::array<u8, 3> seg_pred_probs;
};

struct Vp9FrameContainer {
    Vp9PictureInfo info{};
    std::vector<u8> bit_stream;
};

struct PictureInfo {
    INSERT_PADDING_WORDS_NOINIT(12);
    u32 bitstream_size;
    INSERT_PADDING_WORDS_NOINIT(5);
    Vp9FrameDimensions last_frame_size;
    Vp9FrameDimensions golden_frame_size;
    Vp9FrameDimensions alt_frame_size;
    Vp9FrameDimensions current_frame_size;
    FrameFlags vp9_flags;
    std::array<s8, 4> ref_frame_sign_bias;
    u8 first_level;
    u8 sharpness_level;
    u8 base_q_index;
    u8 y_dc_delta_q;
    u8 uv_ac_delta_q;
    u8 uv_dc_delta_q;
    u8 lossless;
    u8 tx_mode;
    u8 allow_high_precision_mv;
    u8 interp_filter;
    u8 reference_mode;
    INSERT_PADDING_BYTES_NOINIT(3);
    u8 log2_tile_cols;
    u8 log2_tile_rows;
    Segmentation segmentation;
    LoopFilter loop_filter;
    INSERT_PADDING_BYTES_NOINIT(21);

    [[nodiscard]] Vp9PictureInfo Convert() const {
        return {
            .bitstream_size = bitstream_size,
            .ref_frame_sign_bias = ref_frame_sign_bias,
            .base_q_index = base_q_index,
            .y_dc_delta_q = y_dc_delta_q,
            .uv_dc_delta_q = uv_dc_delta_q,
            .uv_ac_delta_q = uv_ac_delta_q,
            .transform_mode = tx_mode,
            .interp_filter = interp_filter,
            .reference_mode = reference_mode,
            .log2_tile_cols = log2_tile_cols,
            .log2_tile_rows = log2_tile_rows,
            .ref_deltas = loop_filter.ref_deltas,
            .mode_deltas = loop_filter.mode_deltas,
            .frame_size = current_frame_size,
            .first_level = first_level,
            .sharpness_level = sharpness_level,
            .is_key_frame = True(vp9_flags & FrameFlags::IsKeyFrame),
            .intra_only = True(vp9_flags & FrameFlags::IntraOnly),
            .last_frame_was_key = True(vp9_flags & FrameFlags::LastFrameIsKeyFrame),
            .error_resilient_mode = True(vp9_flags & FrameFlags::ErrorResilientMode),
            .last_frame_shown = True(vp9_flags & FrameFlags::LastShowFrame),
            .show_frame = true,
            .lossless = lossless != 0,
            .allow_high_precision_mv = allow_high_precision_mv != 0,
            .segment_enabled = segmentation.enabled != 0,
            .mode_ref_delta_enabled = loop_filter.mode_ref_delta_enabled != 0,
            .segmentation = segmentation,
        };
    }
};
static_assert(sizeof(PictureInfo) == 0x100, "PictureInfo is an invalid size");

struct EntropyProbs {
    std::array<u8, 10 * 10 * 8> kf_bmode_prob;
    std::array<u8, 10 * 10 * 1> kf_bmode_probB;
    std::array<u8, 3> ref_pred_probs;
    std::array<u8, 7> mb_segment_tree_probs;
    std::array<u8, 3> segment_pred_probs;
    std::array<u8, 4> ref_scores;
    std::array<u8, 2> prob_comppred;
    INSERT_PADDING_BYTES_NOINIT(9);
    std::array<u8, 10 * 8> kf_uv_mode_prob;
    std::array<u8, 10 * 1> kf_uv_mode_probB;
    INSERT_PADDING_BYTES_NOINIT(6);
    std::array<u8, 28> inter_mode_prob;
    std::array<u8, 4> intra_inter_prob;
    INSERT_PADDING_BYTES_NOINIT(80);
    std::array<u8, 2> tx_8x8_prob;
    std::array<u8, 4> tx_16x16_prob;
    std::array<u8, 6> tx_32x32_prob;
    u8 y_mode_prob_e8[4];
    std::array<std::array<u8, 8>, 4> y_mode_prob_e0e7;
    INSERT_PADDING_BYTES_NOINIT(64);
    std::array<u8, 64> partition_prob;
    INSERT_PADDING_BYTES_NOINIT(10);
    std::array<u8, 8> switchable_interp_prob;
    std::array<u8, 5> comp_inter_prob;
    std::array<u8, 3> skip_probs;
    INSERT_PADDING_BYTES_NOINIT(1);
    std::array<u8, 3> joints;
    std::array<u8, 2> sign;
    std::array<u8, 2> class_0;
    std::array<u8, 6> fr;
    std::array<u8, 2> class_0_hp;
    std::array<u8, 2> high_precision;
    std::array<u8, 20> classes;
    std::array<u8, 12> class_0_fr;
    std::array<u8, 20> pred_bits;
    std::array<u8, 10> single_ref_prob;
    std::array<u8, 5> comp_ref_prob;
    INSERT_PADDING_BYTES_NOINIT(17);
    std::array<u8, 2304> coef_probs;

    void Convert(Vp9EntropyProbs& fc) {
        fc.inter_mode_prob = inter_mode_prob;
        fc.intra_inter_prob = intra_inter_prob;
        fc.tx_8x8_prob = tx_8x8_prob;
        fc.tx_16x16_prob = tx_16x16_prob;
        fc.tx_32x32_prob = tx_32x32_prob;

        for (std::size_t i = 0; i < 4; i++) {
            for (std::size_t j = 0; j < 9; j++) {
                fc.y_mode_prob[j + 9 * i] = j < 8 ? y_mode_prob_e0e7[i][j] : y_mode_prob_e8[i];
            }
        }

        fc.partition_prob = partition_prob;
        fc.switchable_interp_prob = switchable_interp_prob;
        fc.comp_inter_prob = comp_inter_prob;
        fc.skip_probs = skip_probs;
        fc.joints = joints;
        fc.sign = sign;
        fc.class_0 = class_0;
        fc.fr = fr;
        fc.class_0_hp = class_0_hp;
        fc.high_precision = high_precision;
        fc.classes = classes;
        fc.class_0_fr = class_0_fr;
        fc.prob_bits = pred_bits;
        fc.single_ref_prob = single_ref_prob;
        fc.comp_ref_prob = comp_ref_prob;

        for (std::size_t i = 0; i < coef_probs.size(); i += 4) {
            const std::size_t j = i - i / 4;
            fc.coef_probs[j] = coef_probs[i];
            fc.coef_probs[j + 1] = coef_probs[i + 1];
            fc.coef_probs[j + 2] = coef_probs[i + 2];
        }
    }
};
static_assert(sizeof(EntropyProbs) == 0xEA0, "EntropyProbs is an invalid size");

enum class Ref { Last, Golden, AltRef };

struct RefPoolElement {
    s64 frame{};
    Ref ref{};
    bool refresh{};
};

struct VP8PictureInfo {
    INSERT_PADDING_WORDS_NOINIT(14);
    u16 frame_width;
    u16 frame_height;
    u8 key_frame;
    u8 version;
    union {
        u8 raw;
        BitField<0, 2, u8> tile_format;
        BitField<2, 3, u8> gob_height;
        BitField<5, 3, u8> reserved_surface_format;
    };
    u8 error_conceal_on;
    u32 first_part_size;
    u32 hist_buffer_size;
    u32 vld_buffer_size;
    std::array<u32, 2> frame_stride;
    u32 luma_top_offset;
    u32 luma_bot_offset;
    u32 luma_frame_offset;
    u32 chroma_top_offset;
    u32 chroma_bot_offset;
    u32 chroma_frame_offset;

    INSERT_PADDING_BYTES_NOINIT(0x1c);

    s8 current_output_memory_layout;
    std::array<s8, 3> output_memory_layout;
    u8 segmentation_feature_data_update;
    INSERT_PADDING_BYTES_NOINIT(3);

    u32 result_value;
    std::array<u32, 8> partition_offset;
    INSERT_PADDING_WORDS_NOINIT(3);
};
static_assert(sizeof(VP8PictureInfo) == 0xc0, "VP8PictureInfo is an invalid size");

#define ASSERT_POSITION(field_name, position) \
    static_assert(offsetof(Vp9EntropyProbs, field_name) == position)
ASSERT_POSITION(partition_prob, 0x0024);
ASSERT_POSITION(switchable_interp_prob, 0x0724);
ASSERT_POSITION(sign, 0x0772);
ASSERT_POSITION(class_0_fr, 0x079E);
ASSERT_POSITION(high_precision, 0x07B2);
#undef ASSERT_POSITION

#define ASSERT_POSITION(field_name, position) \
    static_assert(offsetof(PictureInfo, field_name) == position)
ASSERT_POSITION(bitstream_size, 0x30);
ASSERT_POSITION(last_frame_size, 0x48);
ASSERT_POSITION(first_level, 0x70);
ASSERT_POSITION(segmentation, 0x80);
ASSERT_POSITION(loop_filter, 0xE4);
#undef ASSERT_POSITION

#define ASSERT_POSITION(field_name, position) \
    static_assert(offsetof(EntropyProbs, field_name) == position)
ASSERT_POSITION(inter_mode_prob, 0x400);
ASSERT_POSITION(tx_8x8_prob, 0x470);
ASSERT_POSITION(partition_prob, 0x4E0);
ASSERT_POSITION(class_0, 0x540);
ASSERT_POSITION(class_0_fr, 0x560);
ASSERT_POSITION(coef_probs, 0x5A0);
#undef ASSERT_POSITION

} // namespace Tegra