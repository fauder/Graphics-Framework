IF "%GLSLANG_PATH%"=="" (
    ECHO Environment variable "GLSLANG_PATH" is not defined. Skipping post-build shader validation.
) ELSE (
    ECHO PostBuild_ValidateShaders.bat: Validating all GLSL vertex and fragment shaders via glslangValidator.exe...
    FOR /R ".\Asset\Shader\" %%G IN  (*.vert, *.frag) DO "%GLSLANG_PATH%"\glslangValidator.exe "%%G"
)