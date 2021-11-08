

import shutil, os

os.makedirs(os.path.dirname("release/pcb/"), exist_ok=True)
os.makedirs(os.path.dirname("release/stencil/"), exist_ok=True)
os.makedirs(os.path.dirname("release/position/"), exist_ok=True)
os.makedirs(os.path.dirname("release/bom/"), exist_ok=True)


shutil.copy2("gerber/TeensyKeyerShield-F_Cu.gbr","release/pcb/cwkeyer.GTL")
shutil.copy2("gerber/TeensyKeyerShield-B_Cu.gbr","release/pcb/cwkeyer.GBL")

shutil.copy2("gerber/TeensyKeyerShield-F_SilkS.gbr","release/pcb/cwkeyer.GTO")
shutil.copy2("gerber/TeensyKeyerShield-B_SilkS.gbr","release/pcb/cwkeyer.GBO")

shutil.copy2("gerber/TeensyKeyerShield-F_Mask.gbr","release/pcb/cwkeyer.GTS")
shutil.copy2("gerber/TeensyKeyerShield-B_Mask.gbr","release/pcb/cwkeyer.GBS")

shutil.copy2("gerber/TeensyKeyerShield.drl","release/pcb/cwkeyer.TXT")

shutil.copy2("gerber/TeensyKeyerShield-Edge_Cuts.gbr","release/pcb/cwkeyer.GML")

shutil.copy2("gerber/TeensyKeyerShield-top.pos","release/position/cwkeyer-top.pos")
shutil.copy2("gerber/TeensyKeyerShield-bottom.pos","release/position/cwkeyer-bot.pos")

shutil.copy2("gerber/TeensyKeyerShield-F_Paste.gbr","release/stencil/cwkeyer-F_Paste.gbr")
shutil.copy2("gerber/TeensyKeyerShield-B_Paste.gbr","release/stencil/cwkwyer-B_Paste.gbr")

shutil.copy2("bom/ibom.html","release/bom/ibom.html")
shutil.copy2("bom/cwkeyer.xlsx","release/bom/cwkeyer.xlsx")

shutil.copy2("TeensyKeyerShield.pdf","release/cwkeyer.pdf")
