-- Animation spritemap for warmill

return {
   build = {
      image = path.dirname(__file__) .. "spritemap.png",
      representative_image = path.dirname(__file__) .. "build_00.png",
      rectangle = {117, 101, 117, 101},
      hotspot = {57, 76},
      regions = {
         {
            rectangle = {0, 0, 114, 101},
            offsets = { {228, 202}, {114, 202}, {0, 202}, {237, 101}, {237, 0} },
         },
      },
   },
   idle = {
      image = path.dirname(__file__) .. "spritemap.png",
      representative_image = path.dirname(__file__) .. "idle_00.png",
      rectangle = {0, 101, 117, 101},
      hotspot = {57, 76},
   },
   unoccupied = {
      image = path.dirname(__file__) .. "spritemap.png",
      representative_image = path.dirname(__file__) .. "unoccupied_00.png",
      rectangle = {120, 0, 117, 101},
      hotspot = {57, 76},
   },
   working = {
      image = path.dirname(__file__) .. "spritemap.png",
      representative_image = path.dirname(__file__) .. "working_00.png",
      rectangle = {0, 0, 120, 101},
      hotspot = {57, 76},
      fps = 10,
      regions = {
         {
            rectangle = {15, 51, 2, 2},
            offsets = { {553, 372}, {553, 372}, {553, 372}, {553, 372}, {553, 372}, {553, 372}, {553, 372}, {551, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {549, 372}, {553, 372}, {553, 372}, {553, 372} },
         },
         {
            rectangle = {66, 94, 1, 1},
            offsets = { {556, 372}, {556, 372}, {556, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {555, 372}, {556, 372} },
         },
         {
            rectangle = {21, 1, 99, 93},
            offsets = { {549, 279}, {549, 186}, {549, 93}, {549, 0}, {396, 396}, {297, 396}, {198, 396}, {99, 396}, {0, 396}, {450, 279}, {450, 186}, {450, 93}, {450, 0}, {297, 303}, {198, 303}, {99, 303}, {0, 303}, {351, 186}, {351, 93}, {351, 0} },
         },
      },
   },
}
