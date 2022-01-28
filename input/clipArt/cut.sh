#!/bin/bash

#512x324
convert clipArtStopLights.png  -trim -crop 165x324+0+0 redLight.png
convert clipArtStopLights.png  -trim -crop 165x324+345+0 greenLight.png
