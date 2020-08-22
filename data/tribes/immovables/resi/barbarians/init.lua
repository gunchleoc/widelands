dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_none",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "No Resources"),
   helptext_script = dirname .. "../helptexts/none.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/none_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "none",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_water",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Water"),
   helptext_script = dirname .. "../helptexts/water.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/water_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "water",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_coal_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Coal"),
   helptext_script = dirname .. "../helptexts/coal_1.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/coal_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "coal_few",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_gold_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Gold"),
   helptext_script = dirname .. "../helptexts/gold_1.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/gold_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "gold_few",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_iron_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Iron"),
   helptext_script = dirname .. "../helptexts/iron_1.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/iron_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "iron_few",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_stones_1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "Some Granite"),
   helptext_script = dirname .. "../helptexts/stones_1.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/stone_few_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "stone_few",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_coal_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Coal"),
   helptext_script = dirname .. "../helptexts/coal_2.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/coal_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "coal_much",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_gold_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Gold"),
   helptext_script = dirname .. "../helptexts/gold_2.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/gold_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "gold_much",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_iron_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Iron"),
   helptext_script = dirname .. "../helptexts/iron_2.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/iron_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "iron_much",
         hotspot = {8, 27}
      }
   }
}

tribes:new_immovable_type {
   msgctxt = "resource_indicator",
   name = "barbarians_resi_stones_2",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("resource_indicator", "A Lot of Granite"),
   helptext_script = dirname .. "../helptexts/stones_2.lua",
   animation_directory = dirname .. "pics",
   icon = dirname .. "pics/stone_much_1.png",
   programs = {
      main = {
         "animate=idle duration:10m",
         "remove="
      }
   },
   animations = {
      idle = {
         basename = "stone_much",
         hotspot = {8, 27}
      }
   }
}
