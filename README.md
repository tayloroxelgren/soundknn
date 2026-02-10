# Purpose
Run knn on audio data which has been converted to images to find similar audio clips

## Ffmpeg preporcessing command to create spectrogram image
Spectogramspectrogram

`for %f in (*.wav) do ffmpeg -y -i "%f" -ar 44100 -lavfi "showspectrumpic=s=256x256:scale=log:legend=0" "img\%~nf.png"`

With loudnorm to try to improve performance by normalizing loudness

<sup> Seems to work much better

`for %f in (*.wav) do ffmpeg -y -i "%f" -ar 44100 -lavfi "loudnorm,showspectrumpic=s=256x256:scale=log:legend=0" "img\%~nf.png"`

## stb_image.h
Used for gathering the image data

## Building
`clang src/main.c -Iexternal -O3 -fopenmp -march=native -o soundknn.exe`

### Todo
- [x] Load all images from directory
- [x] Do knn algorithm
- [ ] Use opencl to accelerate algorithm execution