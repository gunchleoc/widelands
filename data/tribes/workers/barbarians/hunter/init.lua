dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_hunter",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Hunter"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hunting_spear = 1
   },

   programs = {
      hunt = {
         "findobject=type:bob radius:13 attrib:eatable",
         "walk=object",
         "animate=idle 1000",
         "removeobject",
         "createware=meat",
         "return"
      }
   },

   spritesheets = {
      idle = {
         directory = dirname,
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 20 }
      },
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 26 }
      },
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 6, 26 }
      }
   }
}
