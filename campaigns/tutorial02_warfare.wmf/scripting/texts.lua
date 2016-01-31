-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"

-- =============
-- Texts below
-- =============

introduction = {
   title = _"Introduction",
   body = rt(
      h1(_"Soldiers, Training and Warfare") ..
      p(_[[In this scenario, I’m going to tell you about soldiers, their training and their profession: warfare. Although Widelands is about building up, not burning down, there is an enemy you sometimes have to defeat. Yet warfare is mainly focused on economics, not on military strategies, and its mechanics deserve explanation.]]) ..
      p(_[[I’ve set up a small village that contains the most important buildings. You also have enough wares, so you do not have to take care of your weapons production. In a real game, you will not have this luxury.]])
   ),
   h = 300
}

abilities = {
   position = "topright",
   title = _"Soldiers’ abilities",
   body = rt(
      p(_[[A new soldier is created like a worker: when a military building needs a soldier, a carrier grabs the needed weapons and armor from a warehouse (or your headquarters) and walks up the road to your new building. Basic Barbarian soldiers do not use armor, they only need an ax.]]) ..
      p(_[[Take a look at the soldiers that are on their way to our military buildings. They look different from normal workers: they have a health bar over their head that displays their remaining health, and they have four symbols, which symbolize the individual soldier’s current levels in the four different categories: health, attack, defense and evade.]]) ..
      p(_[[If a Barbarian soldier is fully trained, he has level 3 health, level 5 attack, level 0 defense and level 2 evade. This is one fearsome warrior then! The individual abilities have the following meaning:]])
   ) ..
   rt("image=tribes/workers/barbarians/soldier/hp_level0.png", h2(_"Health:"))..
   rt(p(_[[The total life of a soldier. A Barbarian soldier starts with 130 health, and he will gain 28 health with each health level.]])) ..
   rt("image=tribes/workers/barbarians/soldier/attack_level0.png", h2(_"Attack:")) ..
   rt(p(_[[The amount of damage a soldier will inflict on the enemy when an attack is successful. A Barbarian soldier with attack level 0 inflicts ~14 points of health damage when he succeeds in hitting an enemy. For each attack level, he gains 7 damage points.]])) ..
   -- The Atlanteans' image, because the Barbarian one has a white background
   rt("image=tribes/workers/atlanteans/soldier/defense_level0.png", h2(_"Defense:")) ..
   rt(p(_[[The defense is the percentage that is subtracted from the attack value. The Barbarians cannot train in this skill and therefore have always defense level 0, which means that the damage is always reduced by 3%. If an attacker with an attack value of 35 points hits a Barbarian soldier, the Barbarian will lose 35·0.97 = 34 health.]])) ..
   rt("image=tribes/workers/barbarians/soldier/evade_level0.png", h2(_"Evade:")) ..
   rt(p(_[[Evade is the chance that the soldier is able to dodge an attack. A level 0 Barbarian has a 25% chance to evade an attack, and this increases in steps of 15% for each level.]]))
}

battlearena1 = {
   position = "topright",
   title = _"The Battle Arena",
   body = rt(
      p(_[[Now I have talked about training and levels. Let me elaborate on that.]]) ..
      p(_[[A newly created soldier has no experience and is not very good at fighting. To make him stronger, you can build training sites.]]) ..
      p(_[[One of these training sites is the battle arena. It is a big and expensive building, and it trains soldiers in evade. Since soldiers get very hungry during their workout, this building needs a lot of food and strong beer. In a real game, you should have a good infrastructure before you build it.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[To see evade training in action, build a battle arena.]]) ..
      "</p><p font-size=8><br></p>" ..
      p(_[[While we’re waiting for the battle arena, you’ll probably notice some soldiers walking around. They are automatically exchanged from time to time. I’ll teach you about that later.]])
   ),
   h = 400,
   obj_name = "build_battlearena",
   obj_title = _"Build a battle arena",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Build a battle arena. It is a big building.]]) ..
      listitem_arrow(_[[Since the construction will take some time, you can change the game speed using Page Up and Page Down.]])
   )
}

battlearena2 = {
   position = "topright",
   title = _"The Battle Arena",
   body = rt(
      h1(_"The Battle Arena Has Been Constructed") ..
      p(_[[Very good. Our battle arena has been finished, and the soldiers are already walking towards it.]]) ..
      -- Not perfectly correct (some training steps need either bread or meat), but we do not want to confuse new players
      p(_[[The needed wares are also delivered there. For successful training, you need pitta bread and strong beer, as well as either fish or meat.]] .. " " ..
      _[[For more information, you can have a look at the building’s help window, accessible via the question mark in every building’s window.]]) ..
      p(_[[To learn how far your soldiers have progressed in their training, you can have a look at their icons. They are modified by red dots:]])
   ) ..
   rt("image=tribes/workers/barbarians/soldier/evade_level0.png", p(_[[No red dots means that the soldier is not trained, so he has level 0. All your new recruits have this.]])) ..
   rt("image=tribes/workers/barbarians/soldier/evade_level1.png", p(_[[With every successful training step, your soldier becomes stronger. This is indicated by a red dot. This soldier is on level 1 in evade training.]])) ..
   rt("image=tribes/workers/barbarians/soldier/evade_level2.png", p(_[[When your soldier has reached the highest possible level (in this case level 2), this is indicated by a white background color.]])),
   h = 450
}

trainingcamp1 = {
   position = "topright",
   title = _"The Training Camp",
   body = rt(
      h1(_"The Training Camp") ..
      p(_[[There is a second training site: the training camp. It is a big building too, and to complement the battle arena, it trains attack and health (remember, the Barbarian soldiers cannot be trained in defense).]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a training camp.]])
   ),
   h = 300,
   obj_name = "build_trainingcamp",
   obj_title = _"Build a training camp",
   obj_body = rt(
      p(_[[The battle arena only trains the soldiers in evade. To get the strongest possible soldier, you also need to build a training camp, which trains them in attack and health.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a training camp.]])
   )
}

trainingcamp2 = {
   position = "topright",
   title = _"The Training Camp",
   body = rt(
      p(_[[Great, our training camp has now been finished, too. Now nothing will hinder us from getting the strongest warriors the world has ever seen.]]) ..
      p(_[[To train in the training camp, our soldiers need food like in the battle arena, but no strong beer. Instead, they need different axes for attack training and helmets for health training.]]) ..
      p(_[[This equipment is produced in smithies out of coal, iron, and sometimes gold. You will learn more about this in the second scenario of the Barbarian campaign.]]) ..
      p(_[[You should also keep in mind that each of the three tribes in Widelands has its own way of training, so the buildings and wares are different. Also, the ability levels cannot be compared: an Imperial soldier with evade level 0 has a 30% chance of evading, while a Barbarian soldier at the same level only has a 25% chance.]])
   )
}

heroes_rookies = {
   position = "topright",
   title = _"Heroes and Rookies",
   body = rt(
      h1(_"Heroes and Rookies") ..
      p(_[[While our soldiers are training, let me tell you what we can do with them.]]) ..
      p(_[[In every military building, you can set the preference for heroes (trained soldiers) or rookies. From time to time, a soldier will walk out of the building and be replaced by a stronger/weaker one automatically – this is what you saw earlier.]]) ..
      p(_[[The initial setting depends on the type of the building. For the Barbarians, the sentry is the only building that prefers rookies by default. You should change this setting to fit your current needs.]]) ..
      p(_[[When you are expanding into no man’s land, you can make your buildings prefer rookies. When you are planning to attack, send heroes into that region. Conquered buildings always prefer heroes.]])
   )
}

soldier_capacity = {
   position = "topright",
   title = _"Soldier capacity",
   body = rt(
      h1(_"Adjusting the number of soldiers") ..
      p(_[[There is another way how you can control the strength of a military building: by the number of soldiers stationed there. Just click on the arrow buttons to decrease or increase the desired number of soldiers. Every building has a maximum capacity. In case of the barrier, it is five, for example.]]) ..
      p(_[[If you wish to send a certain soldier away, you can simply click on it. It will then be replaced by another soldier.]]) ..
      p(_[[Let me also describe what the numbers in the statistics string mean. This string can contain up to three numbers, e.g. ‘1 (+5) soldier (+2)’.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[The first number describes how many soldiers are currently in this building. In this example, only one soldier is left inside (each military building is always guarded by at least one soldier).]]) ..
      listitem_bullet(_[[The second number tells you how many additional soldiers reside in this building, but are currently outside. The five soldiers may be attacking an enemy. They will return when they have been successful.]]) ..
      listitem_bullet(_[[The third number indicates the missing soldiers. From the eight soldiers (1 + 5 + 2) you wish to have here, two may have died. They will be replaced by new soldiers from your warehouse, if possible.]])
   )
}

dismantle = {
   position = "topright",
   title = _"Dismantle your sentry",
   body = rt(
      h1(_"Dismantling military buildings") ..
      p(_[[You can only reduce the number of soldiers to one. The last soldier of a building will never come out (unless this building is attacked). If you want to have your soldier elsewhere, you will have to dismantle the building (buildings of an alien tribe cannot be dismantled, only be burned down).]]) ..
      p(_[[However, destroying a military building is always linked with a risk: the land is still yours, but it is no longer protected. Any enemy that builds his own military sites can take over that land without a fight, causing your buildings to burst into flames. Furthermore, some parts of the land can now be hidden under the fog of war. You should therefore only dismantle military buildings deep inside your territory where you are safe from enemies.]]) ..
      p(_[[Have you seen your sentry? Since it cannot contain many soldiers and is next to a stronger barrier, it is rather useless.]]) ..
      paragraphdivider() ..
      -- TRANSLATORS: 'it' refers to the Barbarian sentry
      listitem_bullet(_[[Dismantle it.]])
   ) ..
   rt(p(_[[You can also use this opportunity to become familiar with the other options: the heroes/rookies preference and the capacity.]])),
   obj_name = "dismantle_sentry",
   obj_title = _"Dismantle your north-western sentry",
   obj_body = rt(
      p(_[[You can control the number of soldiers stationed at a military site with the arrow buttons. If you want to get even your last soldier out, you will have to destroy it. However, it then will no longer protect your territory, which will make it vulnerable to hostile attacks.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Dismantle your sentry in the north-west, next to the barrier.]])
   )
}

fortress_enhancement = {
   position = "topright",
   title = _"Enhance Your Fortress",
   body = rt(
      h1(_"Enhancing Buildings") ..
      p(_[[Well done. Now you know how to draw back your soldiers from the places where you don’t need them. It is time to tell you how to reinforce your front line.]]) ..
      p(_[[Your fortress is already quite strong and conquers a lot of space. But there is an even bigger building: the citadel.]]) ..
      p(_[[Citadels can’t be built directly. Instead, you’ll have to construct a fortress first and then enhance it to a citadel. To do so, click on the fortress, then choose the ‘Enhance to Citadel’ button.]]) ..
      p(_[[Your soldiers will leave the fortress while the construction is going on. This means that your fortress will lose its military influence, as I described above.]]) ..
      listitem_bullet(_[[Enhance your fortress to a citadel now.]])
   ),
   obj_name = "enhance_fortress",
   obj_title = _"Enhance your fortress to a citadel",
   obj_body = rt(
      h1(_"Enhance Your Fortress") ..
      paragraphdivider() ..
      listitem_bullet(_[[Enhance your fortress to a mighty citadel.]]) ..
      listitem_arrow(_[[The citadel can house 12 soldiers, and it is the biggest military building the Barbarians can build. It also costs a lot of resources and takes a long time to build. It is most suited to guard strategically important points like constricted points or mountains.]])
   )
}

attack_enemy = {
   position = "topright",
   field = wl.Game().map:get_field(29,4), -- show the lost territory
   title = _"Defeat your Enemy",
   body = rt(
      h1(_"Defeat the Enemy") ..
      p(_[[Great work, the citadel is finished. But what’s that? A hostile tribe has settled next to us while the citadel was under construction! Do you see how they took away a part of our land? And our lumberjack has now lost his place of work. This is what I was talking about. Let’s take our land back and defeat the enemy!]]) ..
      p(_[[To attack a building, click on its doors, choose the number of soldiers that you wish to send and click on the ‘Attack’ button.]] .. " " .. _[[Your soldiers will come from all nearby military buildings. Likewise, the defenders will come from all nearby military buildings of the enemy and intercept your forces.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Attack and conquer all military buildings of the enemy and destroy their headquarters.]])
   ),
   h = 350,
   obj_name = "defeated_the_empire",
   obj_title = _"Defeat the enemy tribe",
   obj_body = rt(
      h1(_"Defeat Your Enemy") ..
      paragraphdivider() ..
      listitem_bullet(_[[Defeat the nearby enemy.]]) ..
      listitem_arrow(_[[To attack a building, click on its doors, choose the number of soldiers that you wish to send and click on the ‘Attack’ button.]])
   )
}

conclude_tutorial = {
   title = _"Conclusion",
   body = rt(
      h1(_"Conclusion") ..
      p(_[[Thank you for playing this tutorial. I hope you enjoyed it and you learned how to create and train soldiers, how to control where they go and how to defeat an enemy. Did you see how easily you could overwhelm your enemy? Having trained soldiers is a huge advantage.]]) ..
      p(_[[But a war is expensive, and not always the path leading to the goal. When setting up a new game, you can also choose peaceful win conditions. You should definitely try them out, they’re worth it.]]) ..
      p(_[[You are now ready to play the campaigns. They will teach you about the different economies of the tribes. You can also play the remaining tutorials, but they are not crucial for succeeding in the campaigns.]])
   )
}
