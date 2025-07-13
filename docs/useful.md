# Useful Commands

## Create multiple resolution icon files in one
```
magick convert .\image.png -define icon:auto-resize="256,128,96,64,48,32,24,16" icon.ico
```

## Create ico at specific resolution from png
```
convert image.png -clone 0 -resize 16x16 -delete 0 -alpha remove -colors 256 icon.ico
```

https://usage.imagemagick.org/thumbnails/#favicon
