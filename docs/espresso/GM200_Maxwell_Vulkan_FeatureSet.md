
1. GTX 980 Ti (GM200 Maxwell) Vulkan 1.3 & 1.4 Feature-Set Assessment

While the NVIDIA GeForce GTX 980 Ti is officially Khronos-conformant for both Vulkan 1.3 and Vulkan 1.4 (via NVIDIA's final R580 legacy driver branch), API conformance only requires meeting mandatory baseline specifications—many major architectural features remain either emulated in driver software (partial) or unavailable due to physical 28 nm Maxwell (GM200) silicon limits.

Vulkan 1.3 Feature Breakdown


Feature / Extension	GTX 980 Ti Status	Technical Reality on Maxwell (GM200)
Dynamic Rendering (VK_KHR_dynamic_rendering)	Full	Supported natively in hardware/command processor; removes legacy VkRenderPass and VkFramebuffer object overhead.
Synchronization 2 (VK_KHR_synchronization2)	Full	Supported cleanly in driver; simplifies pipeline barriers and queue submission without hardware penalties.
Maintenance 4 (VK_KHR_maintenance4)	Full	Fully supported; enables cleaner local workgroup sizing and early destruction of shader modules after pipeline creation.
Extended Dynamic State 1 & 2	Partial	Core dynamic states (viewport, scissor, cull mode, depth bounds) work natively, while deeper dynamic state variants trigger internal driver pipeline re-binds.
Graphics Pipeline Libraries (VK_EXT_graphics_pipeline_library)	Partial / Emulated	Exposed by the R580 driver, but Maxwell lacks hardware pre-compiled shader stages. The driver still performs full monolithic pipeline linking on the CPU, resulting in compilation stutter.
Descriptor Indexing / Bindless (VK_EXT_descriptor_indexing)	Partial (Hard-Capped)	Supported up to strict Maxwell hardware descriptor table limits (far lower than Turing/Ampere), risking device loss (VK_ERROR_DEVICE_LOST) if exceeded.


Vulkan 1.4 Feature Breakdown


Feature / Extension	GTX 980 Ti Status	Technical Reality on Maxwell (GM200)
Push Descriptors (VK_KHR_push_descriptor)	Full	Natively supported; allows direct uniform/texture updates inside command buffers without descriptor pool allocation.
Scalar Block Layouts (VK_EXT_scalar_block_layout)	Full	Natively supported; eliminates C-style memory padding overhead when uploading Switch uniform buffers.
Maintenance 5 & 6 (VK_KHR_maintenance5/6)	Full	Fully supported API cleanups, including VK_KHR_map_memory2 and streamlined pipeline flags.
Shader Subgroup Rotate / Float Controls 2	Full	Supported via Maxwell's 32-thread warp shuffle instructions (SHFL) and FP32/FP16 ALU blocks.
Streaming Transfers (VK_EXT_host_image_copy)	Partial	Uses standard DMA copy engines, but lacks modern resizable BAR (ReBAR) / direct host-to-VRAM layout tiling optimizations found on Turing+.
Dynamic Rendering Local Read	Partial / Emulated	Designed for mobile tile-based GPUs; on immediate-mode GM200 silicon, the driver emulates attachment reads via standard global VRAM barriers.


2. Current Eden Espresso Wave [AX] Utilisation Assessment

Eden Espresso Wave [AX] (Master v1.33) is explicitly engineered around the exact hardware boundaries of Maxwell GPUs like the GTX 980 Ti rather than relying on newer Vulkan 1.3/1.4 abstractions that fall back to slow driver emulation.

What Wave [AX] Actively Utilises

• Maxwell-Safe UBO Passthrough: Directly exploits the GTX 980 Ti's native Push Descriptors and Scalar Block Layouts to stream Nintendo Switch Uniform Buffer Objects (UBOs) straight from host memory into command buffers. This bypasses Maxwell's tight descriptor-indexing hardware caps and avoids costly CPU-side buffer repacking.
• Explicit VRAM Capping & Management: Although the GTX 980 Ti has a generous 6 GB GDDR5 frame buffer over a 384-bit bus (336.6 GB/s), Wave [AX] enforces deterministic VRAM budgets and avoids VK_EXT_host_image_copy paths that can thrash PCIe 3.0 bandwidth on Maxwell.
• Zero-Copy Intel QSV Video Offloading: Because the GTX 980 Ti's 1st-Gen NVDEC block lacks hardware decoding for newer video codecs used in Switch cutscenes, Wave [AX] offloads media decoding to the Intel iGPU's Quick Sync Video (QSV) engine via zero-copy Vulkan descriptors—keeping the GM200 core dedicated solely to 3D rasterization.
• Vulkan Dynamic Dispatch & Aggressive Disk Shader Caching: Because the GTX 980 Ti only has partial/software-emulated support for VK_EXT_graphics_pipeline_library, Wave [AX] relies on persistent, pre-compiled disk shader caches paired with 2:3:1 CPU core isolation so asynchronous shader compilation never stalls main emulation threads.

What Wave [AX] Intentionally Avoids (or Leaves Unused)

• Full Bindless / Descriptor Heap Paths: Disabled or clamped to stay safely inside GM200's hardware descriptor table ceilings.
• Vulkan 1.4 Tile-Based Extensions (Dynamic Rendering Local Read): Ignored, as desktop Maxwell cards gain zero bandwidth savings from tile-local attachment reads.
• Native GPU Async Pipeline Linking: Treated as unreliable on Maxwell R580 drivers; Wave [AX] compensates at the OS/CPU layer (core pinning + PGO) rather than trusting the driver to compile pipelines asynchronously without frametime spikes.

Summary Verdict

For a GTX 980 Ti, Eden Espresso Wave [AX] operates near the theoretical efficiency ceiling of GM200 silicon: it actively leverages the genuine hardware-accelerated parts of Vulkan 1.3/1.4 (Dynamic Rendering, Push Descriptors, Scalar Block Layouts) while deliberately routing around Maxwell's incomplete features (Graphics Pipeline Libraries, unbounded Descriptor Indexing, and legacy NVDEC).
When pairing a GTX 980 Ti (GM200 Maxwell) with Eden Espresso Wave [AX], several native hardware capabilities on the GPU—and on the surrounding Coffee Lake platform—are currently either under-utilised due to conservative lowest-common-denominator defaults (built to protect smaller 2 GB/4 GB Maxwell cards like the GTX 960/970) or left on the table by generic Vulkan paths.
Here is an assessment of the native capabilities that can be better utilised:

1. GM200 Hardware Capabilities Under-Utilised by Conservative Defaults


Native Capability	GTX 980 Ti Spec	Current Wave [AX] Behaviour	Opportunity for Better Utilisation
6 GB Unsegmented VRAM & 384-bit Bus	6,144 MB GDDR5 at 336.6 GB/s (no split-pool penalty like the GTX 970's 3.5 + 0.5 GB layout)	Enforces strict, conservative VRAM caps tailored to prevent spills on 2 GB–4 GB Maxwell cards.	Raise VRAM & Texture Cache Ceilings: Increase Eden's texture/buffer pool limits to keep high-resolution textures, mipmaps, and render targets resident in fast local GDDR5 rather than evicting and re-streaming them over PCIe 3.0.
NVIDIA Maxwell Texture Compression (Delta Colour Compression)	3rd-Gen DCC (up to 4:1 lossless bandwidth compression)	Relies on standard Vulkan image layouts without aggressive transient attachment aliasing.	Transient Attachment Memory Aliasing: Reusing VRAM blocks for short-lived depth/stencil buffers allows GM200's Delta Colour Compression to stay warm in the 2 MB L2 cache, boosting effective fill-rate at 1440p/4K docked scaling.
Native FP16 / Half-Precision Packed Math	Supported at 1:64 FP32 rate on GM200 (unlike Tegra X1's 2:1 fast FP16)	Emulates Switch FP16 shaders either via standard VK_KHR_shader_float16_int8 or driver fallback.	Forced FP32 Shader Promotion: Because GM200's FP16 throughput is severely throttled compared to the Switch's Maxwell (GM20B) GPU, explicitly promoting Switch half-precision math to native FP32 avoids Maxwell desktop ALU stalls.
Dedicated Async DMA Copy Engine	Dual Copy Engines on GM200 (independent from the 3D/Compute graphics queue)	Often serialises buffer/texture uploads on the main graphics queue to stay safe on entry-level Maxwell cards.	Dedicated Transfer Queue (VK_QUEUE_TRANSFER_BIT): Offloading Switch texture uploads and ASTC-decoded textures onto GM200's second hardware DMA engine overlaps PCIe transfers with 3D rendering.


2. Vulkan 1.3 / 1.4 Native Features With Headroom Left

• Full VK_KHR_synchronization2 Split Barriers:
	• Maxwell's command processor handles fine-grained pipeline barriers cleanly. Replacing coarse global ALL_COMMANDS barriers with split synchronization2 events allows GM200's 22 SMM units (2,816 CUDA cores) to overlap compute shader work (such as ASTC texture decompression) with vertex/pixel shading rather than draining the entire GPU pipeline.
• Subgroup Shuffle Instructions (VK_KHR_shader_subgroup_vote / arithmetic):
	• GM200 natively supports 32-thread warp-level register shuffles (SHFL) in hardware. Ensuring Eden's SPIR-V shader compiler emits native subgroup reductions instead of shared-memory (LDS) fallbacks cuts register pressure and boosts occupancy on complex Switch shaders.
• Persistent Mapped Host-Visible / Device-Local Memory (BAR1 Aperture):
	• With a 256 MB BAR1 aperture on GM200, small, frequently updated Switch Uniform Buffer Objects (UBOs) and index buffers can be written directly by the CPU into WC (Write-Combined) mapped VRAM—eliminating an extra staging-buffer copy per frame.

3. Platform-Level Synergy (Coffee Lake + GTX 980 Ti)

• Compute Shader ASTC Decoding vs. CPU Decoding:
	• The GTX 980 Ti has massive raw FP32/INT32 shader throughput (~6.0+ TFLOPS) compared to entry-level Maxwell cards, while the Switch uses hardware ASTC textures that GM200 lacks fixed-function hardware for.
	• Opportunity: Offloading ASTC texture decompression to GPU Compute Shaders on the 980 Ti's 2,816 CUDA cores—rather than decoding them on the 6-core Coffee Lake CPU—frees substantial CPU headroom on your 2:3:1 pinned cores while barely denting the 980 Ti's GPU load.

Summary Priority List for a GTX 980 Ti

1. Enable GPU Compute ASTC Decoding (leveraging GM200's 2,816 CUDA cores to protect CPU frame times).
2. Relax Maxwell VRAM Caps in eden_engine.ini / driver settings to utilise the full 6 GB GDDR5 pool.
3. Use Dedicated DMA Transfer Queues & Split Barriers (synchronization2) to overlap texture uploads with 3D rendering.
4. Promote Switch FP16 to Native FP32 where driver half-precision emulation bottlenecks GM200 ALUs.
