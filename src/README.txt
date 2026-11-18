🗂️ Crucial Operational Step: Moving the Profile to the Windows Root Location
For the FlexASIO backend framework to actually recognize and parse this configuration profile automatically when any software opens up a stream, the file must reside directly inside your root user directory profile folder.
You can quickly copy it to its correct operational location right inside your command prompt:
cmd
copy C:\Users\(username)\Desktop\eden\src\FlexASIO.toml C:\Users\(username)\FlexASIO.toml
Use code with caution.
Once the file is copied to C:\Users\(username)\FlexASIO.toml, it becomes globally active. The moment your emulator runs its initialization handshake pass, FlexASIO will automatically intercept the stream, activate its bit-perfect exclusive bypass mode, and route the un-resampled digital PCM frames straight over the USB bus to your HiBy FC3 dongle.