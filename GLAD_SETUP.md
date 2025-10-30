# GLAD Setup Instructions

GLAD is an OpenGL loading library. You need to generate it for OpenGL 4.1 Core profile.

## Option 1: Web Generator (Recommended)

1. Visit: https://glad.dav1d.de/
2. Select:
   - **Language**: C/C++
   - **API gl**: Version 4.1
   - **Profile**: Core
   - **Extensions**: (leave default)
3. Click **GENERATE**
4. Download the ZIP file
5. Extract contents:
   - Copy `include/glad/` to `external/glad/include/glad/`
   - Copy `include/KHR/` to `external/glad/include/KHR/`
   - Copy `src/glad.c` to `external/glad/src/glad.c`

## Option 2: Run Setup Script

```bash
./setup_glad.sh
```

Note: This may fail due to network issues. Use Option 1 if it doesn't work.

## Verify Installation

After setup, you should have:
```
external/glad/
├── include/
│   ├── glad/
│   │   └── glad.h
│   └── KHR/
│       └── khrplatform.h
└── src/
    └── glad.c
```

## Alternative: Use System GLAD

If you have GLAD installed via package manager:
```bash
brew install glad  # macOS
```

Then modify CMakeLists.txt to use system GLAD instead of local copy.
