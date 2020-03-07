dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_farmer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Farmer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      scythe = 1
   },

   programs = {
      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=plant 6000",
         "plant=attrib:seed_wheat",
         "animate=plant 6000",
         "return"
      },
      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvest 10000",
         "callobject=harvest",
         "animate=gather 4000",
         "createware=wheat",
         "return"
      }
   },

   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 8, 17 },
      },
   },
   spritesheets = {
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 14, 18 }
      },
      walkload = {
         directory = dirname,
         basename = "walkload",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 12, 19 }
      },
      plant = {
         directory = dirname,
         basename = "plant",
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 19 }
      },
      harvest = {
         directory = dirname,
         basename = "harvest",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         hotspot = { 17, 21 }
      },
      gather = {
         directory = dirname,
         basename = "gather",
         fps = 5,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 19 }
      }
   }
}
