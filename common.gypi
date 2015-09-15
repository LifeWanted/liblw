{
    "variables": {
        "coverage%": 0
    },
    "target_defaults": {
        "default_configuration": "Debug",
        "conditions": [
            ["coverage", {
                "cflags": ["--coverage"],
                "xcode_settings": {
                    "OTHER_CPLUSPLUSFLAGS": ["--coverage"]
                }
            }]
        ],
        "configurations": {
            "Debug": {
                "defines": [
                    "DEBUG"
                ],
                "xcode_settings": {
                    "GCC_OPTIMIZATION_LEVEL": "0",
                    "GCC_GENERATE_DEBUGGING_SYMBOLS": "YES",
                    "ARCHS": "$(ARCHS_STANDARD_64_BIT)"
                },
                "msvs_configuration_platform": "x64",
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 3,  # MultiThreadedDebugDLL
                    },
                    "VCLinkerTool": {
                        "GenerateDebugInformation": "true",
                    }
                }
            },
            "Release": {
                "defines": [
                    "NDEBUG"
                ],
                "xcode_settings": {
                    "GCC_OPTIMIZATION_LEVEL": "3",
                    "GCC_GENERATE_DEBUGGING_SYMBOLS": "NO",
                    "DEAD_CODE_STRIPPING": "YES",
                    "GCC_INLINES_ARE_PRIVATE_EXTERN": "YES",
                    "ARCHS": "$(ARCHS_STANDARD_64_BIT)"
                },
                "msvs_configuration_platform": "x64",
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 2,  # MultiThreadedDLL
                    },
                    "VCLinkerTool": {
                        "GenerateDebugInformation": "false",
                    }
                }
            }
        }
    }
}
