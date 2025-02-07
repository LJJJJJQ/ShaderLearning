import os
import platform

vulkanPath = "D:\C_Library\VulkanSDK"
compile_command = "glslc -c "
test_command = compile_command + "--version"

filePath = os.path.abspath(".") + "/shader/"

# Windows platform
# Check glslc command
if platform.system().lower() == "windows":
    if os.system(test_command) != 0:

        test_command = filePath + "glslc.exe -c "
        if os.system(test_command) != 0:

            # Not set global command
            os.system("cls")
            compile_command = vulkanPath + "\Bin\glslc.exe -c "
            test_command = compile_command + "--version "
            
            if os.system(test_command) != 0:
                # Not set right vulkan path
                os.system("cls")
                print("Please set vulkan path!")
                quit()

print("Shader direction: " + filePath + "\n")
failedShaders = []

for root, dirs, files in os.walk(filePath):
    for file in files:
        filename = os.path.splitext(file)
        if filename[1] != ".spv" and filename[1] != ".exe":
            shader_type = filename[1].split('.')[1]

            if filename[1] != ".hlsl":
                ## Record compile command
                cmd = compile_command + filePath + "glsl/" + file + " -o " + filePath + "glsl/" + filename[0] + "_" + shader_type + ".spv"
                print("Compiling " + filename[0] + filename[1])

                ## Execute compile command
                if os.system(cmd) != 0:
                    failedShaders.append(file)

            else:
                sub_filename = os.path.splitext(filename[0])
                shader_type = sub_filename[1].split('.')[1]

                ## Record compile command
                cmd = compile_command + " -c -fshader-stage=" + shader_type + " -fentry-point=main " + filePath \
                + "hlsl/" + file + " -o " + filePath + "hlsl/" + filename[0] + ".spv"
                print("Compiling " + filename[0] + filename[1])

                ## Execute compile command
                if os.system(cmd) != 0:
                    failedShaders.append(file)
                        

if len(failedShaders) == 0:
    print("\nAll shaders compiled successful...")
else:
    print("\nComile failed shaders:")
    for failShader in failedShaders:
        print(failShader)
