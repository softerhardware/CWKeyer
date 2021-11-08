

import shutil, os

os.makedirs(os.path.dirname("release/pcb/"), exist_ok=True)

shutil.copy2("gerber/TeensyKeyerFrontPanel-F_Cu.gbr","release/pcb/cwkeyer_frontpanel.GTL")
shutil.copy2("gerber/TeensyKeyerFrontPanel-B_Cu.gbr","release/pcb/cwkeyer_frontpanel.GBL")

shutil.copy2("gerber/TeensyKeyerFrontPanel-F_SilkS.gbr","release/pcb/cwkeyer_frontpanel.GTO")
shutil.copy2("gerber/TeensyKeyerFrontPanel-B_SilkS.gbr","release/pcb/cwkeyer_frontpanel.GBO")

shutil.copy2("gerber/TeensyKeyerFrontPanel-F_Mask.gbr","release/pcb/cwkeyer_frontpanel.GTS")
shutil.copy2("gerber/TeensyKeyerFrontPanel-B_Mask.gbr","release/pcb/cwkeyer_frontpanel.GBS")

shutil.copy2("gerber/TeensyKeyerFrontPanel.drl","release/pcb/cwkeyer_frontpanel.TXT")

shutil.copy2("gerber/TeensyKeyerFrontPanel-Edge_Cuts.gbr","release/pcb/cwkeyer_frontpanel.GML")
