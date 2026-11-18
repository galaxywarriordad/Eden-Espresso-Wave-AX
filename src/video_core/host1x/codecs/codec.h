// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-FileCopyrightText: License GPL-3.0-or-later

#pragma once

#include <array>
#include <vector>
#include <type_traits>
#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"

namespace Tegra {
namespace Decoders {

struct Offset {
    constexpr u32 Address() const noexcept { return offset << 8; }
private:
    u32 offset;
};
static_assert(std::is_trivial_v<Offset>, "Offset must be trivial");
static_assert(sizeof(Offset) == 0x4, "Offset has the wrong size!");

struct H264ParameterSet {
    s32 log2_max_pic_order_cnt_lsb_minus4; ///< 0x00
    // ... [Fields from 0x04 to 0x54 include picture dimensions, bitfields, pitch, and memory offsets] ...
    u32 hist_buffer_size;                 ///< 0x54
    union {                               ///< 0x58
        union {
            BitField<0, 1, u64> mbaff_frame;
            BitField<1, 1, u64> direct_8x8_inference;
            // ... [Additional bitfield flags up to 63] ...
            BitField<63, 1, u64> output_memory_layout;
        } flags;
        // ... [Additional union bitfields for qp, frame numbers, etc.] ...
    };
};
static_assert(sizeof(H264ParameterSet) == 0x60, "H264ParameterSet is an invalid size");

#define ASSERT_POSITION(field_name, position) \
    static_assert(offsetof(H264ParameterSet, field_name) == position, "Field " #field_name " has invalid position")

ASSERT_POSITION(log2_max_pic_order_cnt_lsb_minus4, 0x00);
ASSERT_POSITION(flags, 0x58);
#undef ASSERT_POSITION

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

// Defines reference frame pool entries with bitfields for indexing, states, and markers.
struct DpbEntry {
    // ... BitField union configuration (index, state, is_long_term, etc.) ...
    std::array<u32, 2> field_order_cnt;
    u32 frame_idx;
};
static_assert(sizeof(DpbEntry) == 0x10, "DpbEntry has the wrong size!");

// Defines display and output scaling parameters, tiling formats, and histogram settings.
struct DisplayParam {
    // ... BitField union configurations and output scaling arrays ...
};
static_assert(sizeof(DisplayParam) == 0x1C, "DisplayParam has the wrong size!");

// Master H.264 decoder context structure tracking streams, slice counts, and scaling matrices.
struct H264DecoderContext {
    INSERT_PADDING_WORDS_NOINIT(13);                        ///< 0x0000
    std::array<u8, 16> eos;                                 ///< 0x0034
    u8 explicit_eos_present_flag;                           ///< 0x0044
    u8 hint_dump_en;                                        ///< 0x0045
    INSERT_PADDING_BYTES_NOINIT(2);                         ///< 0x0046
    u32 stream_len;                                         ///< 0x0048
    u32 slice_count;                                        ///< 0x004C
    u32 mbhist_buffer_size;                                 ///< 0x0050
    u32 gptimer_timeout_value;                              ///< 0x0054
    H264ParameterSet h264_parameter_set;                    ///< 0x0058
    std::array<s32, 2> curr_field_order_cnt;                ///< 0x00B8
    std::array<DpbEntry, 16> dpb;                           ///< 0x00C0
    std::array<u8, 0x60> weight_scale_4x4;                  ///< 0x01C0
    std::array<u8, 0x80> weight_scale_8x8;                  ///< 0x0220
    std::array<u8, 2> num_inter_view_refs_lX;               ///< 0x02A0
    std::array<u8, 14> reserved2;                           ///< 0x02A2
    std::array<std::array<s8, 16>, 2> inter_view_refidx_lX; ///< 0x02B0
    // ... Lossless filter flags and display parameters ...
    DisplayParam display_param;   ///< 0x02D4
    std::array<u32, 3> reserved4; ///< 0x02F0
};
static_assert(sizeof(H264DecoderContext) == 0x2FC, "H264DecoderContext is an invalid size");

#define ASSERT_POSITION(field_name, position)                                                      \
    static_assert(offsetof(H264DecoderContext, field_name) == position,                            \
                  "Field " #field_name " has invalid position")

ASSERT_POSITION(stream_len, 0x48);
ASSERT_POSITION(h264_parameter_set, 0x58);
ASSERT_POSITION(dpb, 0xC0);
ASSERT_POSITION(weight_scale_4x4, 0x1C0);
#undef ASSERT_POSITION

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

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
static_assert(sizeof(Vp9FrameDimensions) == 0x8, "Vp9Vp9FrameDimensions is an invalid size");

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
    constexpr bool operator==(const Segmentation& rhs) const = default;

    u8 enabled;
    u8 update_map;
    u8 temporal_update;
    u8 abs_delta;
    std::array<std::array<u8, 4>, 8> feature_enabled;
    std::array<std::array<s16, 4>, 8> feature_data;
};
static_assert(sizeof(Segmentation) == 0x64, "Segmentation is an invalid size");

struct LoopFilter {
    u8 mode_ref_delta_enabled;
    std::array<s8, 4> ref_deltas;
    std::array<s8, 2> mode_deltas;
};
static_assert(sizeof(LoopFilter) == 0x7, "LoopFilter is an invalid size");

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

// Defines the 0x7B4-byte VP9 entropy probability structures and static size assertion
struct Vp9EntropyProbs {
    std::array<u8, 36> y_mode_prob;           // 0x0000 - 0x0023
    std::array<u8, 64> partition_prob;        // 0x0024 - 0x0063
    std::array<u8, 1728> coef_probs;          // 0x0064 - 0x0723
    // ... (remaining probability arrays from y_mode_prob to high_precision totaling 0x7B4 bytes)
    std::array<u8, 2> high_precision;         // 0x07B2 - 0x07B3
};
static_assert(sizeof(Vp9EntropyProbs) == 0x7B4, "Vp9EntropyProbs is an invalid size");

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

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

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

struct PictureInfo {
    // Inserts hardware register padding and frame structures for VP9 decoding
    INSERT_PADDING_WORDS_NOINIT(12);
    u32 bitstream_size;
    INSERT_PADDING_WORDS_NOINIT(5);
    Vp9FrameDimensions last_frame_size;
    Vp9FrameDimensions golden_frame_size;
    Vp9FrameDimensions alt_frame_size;
    Vp9FrameDimensions current_frame_size;
    FrameFlags vp9_flags;
    std::array<s8, 4> ref_frame_sign_bias;
    u8 first_level, sharpness_level, base_q_index;
    u8 y_dc_delta_q, uv_ac_delta_q, uv_dc_delta_q;
    u8 lossless, tx_mode, allow_high_precision_mv;
    u8 interp_filter, reference_mode;
    INSERT_PADDING_BYTES_NOINIT(3);
    u8 log2_tile_cols, log2_tile_rows;
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

} // namespace Decoders
} // namespace Tegra
namespace Tegra {
namespace Decoders {

struct EntropyProbs {
    std::array<u8, 10 * 10 * 8> kf_bmode_prob;         ///< 0x0000
    std::array<u8, 10 * 10 * 1> kf_bmode_probB;        ///< 0x0320
    std::array<u8, 3> ref_pred_probs;                  ///< 0x0384
    std::array<u8, 7> mb_segment_tree_probs;           ///< 0x0387
    std::array<u8, 3> segment_pred_probs;              ///< 0x038E
    std::array<u8, 4> ref_scores;                      ///< 0x0391
    std::array<u8, 2> prob_comppred;                   ///< 0x0395
    INSERT_PADDING_BYTES_NOINIT(9);                    ///< 0x0397
    std::array<u8, 10 * 8> kf_uv_mode_prob;            ///< 0x03A0
    std::array<u8, 10 * 1> kf_uv_mode_probB;           ///< 0x03F0
    INSERT_PADDING_BYTES_NOINIT(6);                    ///< 0x03FA
    std::array<u8, 28> inter_mode_prob;                ///< 0x0400
    std::array<u8, 4> intra_inter_prob;                ///< 0x041C
    INSERT_PADDING_BYTES_NOINIT(80);                   ///< 0x0420
    std::array<u8, 2> tx_8x8_prob;                     ///< 0x0470
    std::array<u8, 4> tx_16x16_prob;                   ///< 0x0472
    std::array<u8, 6> tx_32x32_prob;                   ///< 0x0476
    std::array<u8, 4 * 9> y_mode_prob_e0e7;            ///< 0x047C
    std::array<u8, 4> y_mode_prob_e8;                  ///< 0x04A0
    INSERT_PADDING_BYTES_NOINIT(60);                   ///< 0x04A4
    std::array<u8, 64> partition_prob;                 ///< 0x04E0
    std::array<u8, 8> switchable_interp_prob;          ///< 0x0520
    std::array<u8, 5> comp_inter_prob;                 ///< 0x0528
    std::array<u8, 3> skip_probs;                      ///< 0x052D
    std::array<u8, 3> joints;                          ///< 0x0530
    std::array<u8, 2> sign;                            ///< 0x0533
    std::array<u8, 2> class_0;                         ///< 0x0535
    std::array<u8, 6> fr;                              ///< 0x0537
    std::array<u8, 2> class_0_hp;                      ///< 0x053D
    std::array<u8, 2> high_precision;                  ///< 0x053F
    std::array<u8, 20> classes;                        ///< 0x0541
    std::array<u8, 12> class_0_fr;                     ///< 0x0555
    std::array<u8, 20> pred_bits;                      ///< 0x0561
    std::array<u8, 10> single_ref_prob;                ///< 0x0575
    std::array<u8, 5> comp_ref_prob;                   ///< 0x057F
    INSERT_PADDING_BYTES_NOINIT(27);                   ///< 0x0585
    std::array<u8, 2304> coef_probs;                   ///< 0x05A0

    void Convert(Vp9EntropyProbs& fc) const {
        fc.inter_mode_prob = inter_mode_prob;
        fc.intra_inter_prob = intra_inter_prob;
        fc.tx_8x8_prob = tx_8x8_prob;
        fc.tx_16x16_prob = tx_16x16_prob;
        fc.tx_32x32_prob = tx_32x32_prob;
        
        // Flatten matrix dimensions into system parameters
        for (std::size_t i = 0; i < 4; i++) {
            for (std::size_t j = 0; j < 9; j++) {
                fc.y_mode_prob[j + 9 * i] = j < 8 ? y_mode_prob_e0e7[j + 8 * i] : y_mode_prob_e8[i];
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

// =======================================================================
// SYSTEM TYPE RESOLUTION VALIDATION CHECKPOINTS
// =======================================================================
#define ASSERT_POSITION(field_name, position) static_assert(offsetof(Vp9EntropyProbs, field_name) == position)
ASSERT_POSITION(partition_prob, 0x0024);
ASSERT_POSITION(switchable_interp_prob, 0x0724);
ASSERT_POSITION(sign, 0x0772);
ASSERT_POSITION(class_0_fr, 0x079E);
ASSERT_POSITION(high_precision, 0x07B2);
#undef ASSERT_POSITION

#define ASSERT_POSITION(field_name, position) static_assert(offsetof(PictureInfo, field_name) == position)
ASSERT_POSITION(bitstream_size, 0x30);
ASSERT_POSITION(last_frame_size, 0x48);
ASSERT_POSITION(first_level, 0x70);
ASSERT_POSITION(segmentation, 0x80);
ASSERT_POSITION(loop_filter, 0xE4);
#undef ASSERT_POSITION

#define ASSERT_POSITION(field_name, position) static_assert(offsetof(EntropyProbs, field_name) == position)
ASSERT_POSITION(inter_mode_prob, 0x400);
ASSERT_POSITION(tx_8x8_prob, 0x470);
ASSERT_POSITION(partition_prob, 0x4E0);
ASSERT_POSITION(fr, 0x537);
ASSERT_POSITION(class_0_fr, 0x555);
ASSERT_POSITION(coef_probs, 0x5A0);
#undef ASSERT_POSITION

} // namespace Decoders
} // namespace Tegra
