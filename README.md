# Purpose
Run knn on audio data which has been converted to images to find similar audio clips

## Ffmpeg preporcessing command to create spectrogram image
`for %f in (*.wav) do ffmpeg -y -i "%f" -ar 44100 -lavfi "showspectrumpic=s=256x256:scale=log:legend=0" "img\%~nf.png"`

## stb_image.h
Used for gathering the image data

## Building
`clang src/main.c -Iexternal -O2 -o soundknn.exe`

### Todo
- [x] Load all images from directory
- [ ] Do knn algorithm
- [ ] Use opencl to accelerate algorithm execution