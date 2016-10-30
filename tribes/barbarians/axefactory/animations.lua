-- Animation spritemap for axefactory

return {
   build = {
      spritemap = "axefactory.png",
      size = {117, 89},
      offset = {117, 100},
      hotspot = {57, 76},
   },
   idle = {
      spritemap = "axefactory.png",
      size = {117, 89},
      offset = {0, 100},
      hotspot = {57, 76},
   },
   unoccupied = {
      spritemap = "axefactory.png",
      size = {117, 89},
      offset = {117, 0},
      hotspot = {57, 76},
   },
   working = {
      spritemap = "axefactory.png",
      size = {117, 100},
      offset = {0, 0},
      hotspot = {57, 76},
      fps = 10,
      regions = {
         {
            dimensions = {89, 0, 23, 25},
            offsets = { {161, 302}, {138, 302}, {115, 302}, {92, 302}, {69, 302}, {46, 302}, {23, 302}, {0, 302}, {303, 162}, {280, 162}, {257, 162}, {234, 162}, {328, 271}, {328, 246}, {328, 221}, {328, 196}, {328, 171}, {328, 146}, {328, 121}, {328, 96} },
         },
         {
            dimensions = {89, 25, 1, 4},
            offsets = { {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {362, 96}, {360, 101}, {360, 101}, {360, 101}, {361, 96}, {360, 96}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101}, {360, 101} },
         },
         {
            dimensions = {79, 61, 14, 13},
            offsets = { {310, 302}, {296, 302}, {282, 302}, {268, 302}, {254, 302}, {240, 302}, {226, 302}, {212, 302}, {198, 302}, {184, 302}, {314, 253}, {300, 253}, {286, 253}, {272, 253}, {258, 253}, {244, 253}, {230, 253}, {216, 253}, {202, 253}, {188, 253} },
         },
         {
            dimensions = {41, 63, 1, 1},
            offsets = { {362, 137}, {362, 137}, {362, 137}, {362, 137}, {362, 137}, {362, 137}, {362, 137}, {362, 137}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 136}, {362, 137}, {362, 137}, {362, 137} },
         },
         {
            dimensions = {42, 64, 1, 1},
            offsets = { {362, 135}, {362, 135}, {362, 135}, {362, 135}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 134}, {362, 135}, {362, 135} },
         },
         {
            dimensions = {35, 65, 5, 4},
            offsets = { {356, 189}, {356, 189}, {356, 189}, {356, 189}, {351, 189}, {351, 189}, {356, 185}, {356, 185}, {351, 185}, {351, 185}, {356, 181}, {356, 181}, {356, 181}, {356, 181}, {356, 181}, {356, 181}, {351, 185}, {351, 181}, {356, 189}, {356, 189} },
         },
         {
            dimensions = {40, 65, 2, 1},
            offsets = { {360, 139}, {360, 139}, {360, 139}, {360, 135}, {360, 135}, {360, 134}, {360, 134}, {360, 134}, {360, 130}, {360, 130}, {360, 130}, {360, 130}, {360, 129}, {360, 129}, {360, 129}, {360, 129}, {360, 130}, {360, 134}, {360, 135}, {360, 139} },
         },
         {
            dimensions = {33, 69, 1, 1},
            offsets = { {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 130}, {362, 129}, {362, 129}, {362, 129}, {362, 129}, {362, 130}, {362, 130}, {362, 130}, {362, 130} },
         },
         {
            dimensions = {34, 69, 4, 5},
            offsets = { {351, 241}, {351, 241}, {359, 236}, {355, 236}, {351, 236}, {359, 231}, {359, 231}, {355, 231}, {351, 231}, {359, 226}, {355, 226}, {351, 226}, {359, 221}, {355, 221}, {355, 221}, {351, 226}, {351, 221}, {359, 231}, {359, 216}, {351, 241} },
         },
         {
            dimensions = {100, 69, 3, 2},
            offsets = { {360, 124}, {360, 124}, {360, 124}, {360, 124}, {360, 119}, {360, 114}, {360, 114}, {360, 114}, {360, 114}, {360, 114}, {360, 114}, {360, 109}, {360, 109}, {360, 109}, {360, 109}, {360, 109}, {360, 114}, {360, 114}, {360, 124}, {360, 124} },
         },
         {
            dimensions = {29, 71, 4, 5},
            offsets = { {355, 216}, {355, 216}, {351, 216}, {359, 211}, {359, 211}, {355, 211}, {351, 211}, {359, 206}, {355, 206}, {351, 206}, {359, 201}, {355, 201}, {351, 201}, {359, 196}, {359, 196}, {355, 196}, {351, 196}, {351, 211}, {359, 211}, {355, 216} },
         },
         {
            dimensions = {34, 75, 4, 3},
            offsets = { {355, 249}, {355, 249}, {355, 249}, {355, 249}, {351, 249}, {359, 246}, {355, 246}, {351, 246}, {359, 241}, {355, 241}, {355, 241}, {359, 193}, {355, 193}, {355, 193}, {355, 193}, {359, 193}, {355, 241}, {351, 193}, {355, 249}, {355, 249} },
         },
         {
            dimensions = {29, 77, 3, 3},
            offsets = { {360, 131}, {360, 131}, {360, 131}, {360, 131}, {360, 126}, {360, 121}, {360, 116}, {360, 116}, {360, 111}, {360, 106}, {360, 106}, {360, 106}, {360, 106}, {360, 106}, {360, 106}, {360, 106}, {360, 111}, {360, 116}, {360, 131}, {360, 131} },
         },
         {
            dimensions = {79, 79, 1, 1},
            offsets = { {362, 105}, {362, 105}, {362, 105}, {362, 105}, {362, 105}, {362, 105}, {362, 105}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 100}, {362, 105}, {362, 105}, {362, 105} },
         },
         {
            dimensions = {75, 82, 2, 3},
            offsets = { {360, 146}, {360, 146}, {360, 146}, {360, 146}, {360, 146}, {360, 146}, {360, 141}, {360, 141}, {360, 136}, {360, 136}, {361, 101}, {361, 101}, {361, 101}, {361, 101}, {361, 101}, {361, 101}, {360, 136}, {360, 141}, {360, 146}, {360, 146} },
         },
         {
            dimensions = {77, 84, 2, 1},
            offsets = { {360, 105}, {360, 105}, {360, 105}, {360, 105}, {360, 105}, {360, 105}, {360, 105}, {361, 104}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 100}, {360, 105}, {360, 105}, {360, 105} },
         },
         {
            dimensions = {43, 43, 35, 32},
            offsets = { {328, 64}, {328, 32}, {328, 0}, {280, 270}, {245, 270}, {210, 270}, {175, 270}, {140, 270}, {105, 270}, {70, 270}, {35, 270}, {0, 270}, {293, 221}, {258, 221}, {223, 221}, {188, 221}, {293, 189}, {258, 189}, {223, 189}, {188, 189} },
         },
         {
            dimensions = {83, 80, 9, 5},
            offsets = { {351, 176}, {351, 171}, {351, 166}, {351, 161}, {351, 156}, {351, 151}, {351, 146}, {351, 141}, {351, 136}, {351, 131}, {351, 126}, {351, 121}, {351, 116}, {351, 111}, {351, 111}, {351, 121}, {351, 106}, {351, 101}, {351, 96}, {351, 171} },
         },
      },
   },
}
