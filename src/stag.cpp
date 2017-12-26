#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
// #include "graphics/2d_primitives.h"
#include "graphics/shader.h"
#include "graphics/canvas.h"
#include <vector>
using namespace std;
using namespace demoloop;

const std::string svg[] = {
  "M 410,1065.8079",
  "C 384.76098,1063.2534 354.98029,1053.4212 332.22026,1040.1286",
  "C 315.82128,1030.5511 298.74423,1015.2153 287.0191,999.5364",
  "C 276.19664,985.06457 265.7753,962.36204 261.52572,944",
  "L 259.78998,936.5",
  "L 271.01119,913",
  "C 283.97955,885.84103 308.70697,832.89765 311.94844,825.35008",
  "L 314.16018,820.20015",
  "L 310.08984,789.35008",
  "C 303.7151,741.03455 302.70012,728.43797 303.33355,705.5",
  "C 303.63731,694.5 304.18142,684.30205 304.54269,682.8379",
  "L 305.19955,680.1758",
  "L 298.76714,682.51064",
  "C 295.22932,683.7948 288.54692,685.36134 283.91737,685.99184",
  "C 279.28782,686.62234 271.30814,687.7821 266.18476,688.5691",
  "C 261.06138,689.35609 254.48282,690 251.56575,690",
  "C 239.41828,690 216.45939,684.37004 204.14957,678.37264",
  "C 183.87627,668.4954 159.62875,634.07482 138.02115,584.5",
  "C 132.88818,572.72331 127,557.57017 127,556.13725",
  "C 127,555.73778 139.7125,557.09661 155.25,559.15689",
  "C 170.7875,561.21716 194.075,564.06784 207,565.49172",
  "C 219.925,566.9156 233.65,568.79583 237.5,569.67",
  "C 254.40519,573.50846 283.22128,584.46106 291.01393,590.00992",
  "C 292.65627,591.17937 294,591.87056 294,591.5459",
  "C 294,590.26877 285.5535,574.20443 282.27609,569.24826",
  "C 280.36601,566.35981 278.18734,561.95013 277.43461,559.44897",
  "C 276.68187,556.94782 275.13008,553.58705 273.98618,551.9806",
  "C 270.35628,546.88287 253.1623,534.82445 231.99615,522.53232",
  "C 164.74587,483.47704 106.63025,447.31642 85,431.06857",
  "C 80.875,427.97002 73.538934,422.79145 68.697631,419.56064",
  "C 50.516245,407.42742 31.892187,389.42059 20.214349,372.68418",
  "C 13.931805,363.68018 6.435014,347.72023 3.922314,338",
  "C 1.988679,330.51985 1.796314,327.88024 2.319035,316",
  "C 3.569509,287.57967 11.75832,256.96818 23.585901,236.5",
  "L 28.208718,228.5",
  "L 28.453561,196.5",
  "C 28.70785,163.26556 29.99246,146.45491 34.108377,122.5",
  "C 38.065769,99.46772 46.050322,71.491417 52.749581,57.184893",
  "C 56.020871,50.198927 65.123976,40.820147 74.554676,34.719458",
  "C 88.481269,25.710391 99.50572,21.837335 90.80232,29.011418",
  "C 73.893253,42.949296 60.930062,90.374474 58.469161,147.30062",
  "L 57.855327,161.5",
  "L 66.016527,146",
  "C 70.505188,137.475 76.581587,126.41544 79.519637,121.42321",
  "C 85.025582,112.0677 93.28336,101.99147 94.48305,103.1647",
  "C 94.85687,103.53029 92.23493,115.68029 88.656511,130.1647",
  "C 76.744787,178.3799 68.941863,205.46869 64.663075,213.46079",
  "C 62.725785,217.07934 62.510121,218.95802 62.592262,231.5",
  "C 62.752192,255.91959 67.795158,285.78663 77.0287,317",
  "C 83.30444,338.21472 89.303507,361.27996 90.51316,368.84512",
  "C 92.12683,378.93702 120.76159,406.70402 139.25,416.10502",
  "C 143.35607,418.19288 145,418.6377 145,417.66088",
  "C 145,413.12504 135.41235,366.38206 132.91355,358.73546",
  "C 129.38089,347.92508 122.49995,333.04045 118.03533,326.55134",
  "C 107.46119,311.18237 102.44759,289.26302 101.32632,253.5",
  "C 100.67639,232.77029 102.247,198.00382 104.51657,182.88178",
  "C 110.08312,145.79217 122.52655,119.14122 140.77655,105.22127",
  "C 142.57939,103.84618 144.3032,102.96986 144.60724,103.2739",
  "C 145.69789,104.36456 141.85091,116.60514 138.50363,122.69476",
  "C 131.6116,135.23328 126.95103,151.97868 125.05933,171",
  "C 123.68415,184.82774 125.41346,252.69096 127.46668,265.47119",
  "C 130.79332,286.17782 136.7866,307.59995 143.42165,322.5",
  "C 148.33759,333.53953 157.05354,347.91669 158.49485,347.3636",
  "C 160.64004,346.54042 162.56643,341.50472 166.47614,326.5",
  "C 173.29423,300.33355 176.75393,291.13159 181.79711,285.75",
  "C 184.54547,282.81721 189.00656,282.07079 190.76208,284.25",
  "C 195.0539,289.57763 191.7576,309.62568 180.53553,346.44773",
  "C 175.02736,364.52124 174.01511,370.35097 174.01511,384",
  "C 174.01511,397.35618 175.26574,405.07552 179.39332,417.19632",
  "C 183.95175,430.58228 191.42784,442.21415 203,453.9254",
  "C 221.56607,472.71461 238.79901,482.21385 265.96322,488.63235",
  "C 271.72406,489.99355 277.95395,492.04449 279.80741,493.18999",
  "C 286.85598,497.54625 294.61818,495.15139 298.45898,487.43547",
  "C 300.12272,484.09311 300.38531,481.69484 300.20131,471.52229",
  "C 299.89539,454.60973 303.17775,442.37134 315.24076,415.44702",
  "C 317.6919,409.97616 320.77796,402.65529 322.09869,399.17843",
  "C 328.97827,381.06761 335.13651,389.89989 330.0711,410.61262",
  "C 326.76306,424.13933 325.67589,436.98652 326.28209,455.38781",
  "C 326.79426,470.93476 329.44018,491.626 331.69073,497.68355",
  "C 333.12973,501.55676 377.00185,545.08458 381.96796,547.56621",
  "C 390.80227,551.98083 399.19674,554.22782 407.11659,554.29787",
  "C 417.17902,554.38687 421.91047,551.99784 437.3606,539.02687",
  "C 445.97364,531.79589 451.83082,527.79153 458.75193,524.40231",
  "C 469.58921,519.09539 479.93173,509.80613 483.72589,501.97164",
  "C 486.57534,496.08786 488.89784,474.06341 489.68088,445.5",
  "C 490.79108,405.00261 487.39027,389.04974 474.66844,375.07826",
  "L 470.64219,370.65651",
  "L 476.5711,365.35252",
  "C 481.42477,361.01043 483.16616,360.04413 486.17327,360.02426",
  "C 491.1935,359.99106 495.30305,363.66517 499.44087,371.8859",
  "C 510.15523,393.17243 514.9866,420.23567 516.63373,468.1929",
  "L 517.3101,487.88581",
  "L 519.90505,488.88038",
  "C 530.15662,492.80952 543.71454,490.02524 565.8549,479.44404",
  "C 579.14123,473.0943 584.84321,469.75302 602.66716,457.87256",
  "C 609.9091,453.04549 617.3341,448.4685 619.16716,447.70149",
  "C 634.16038,441.42779 653.77627,401.92692 672.14505,341.01896",
  "L 677.11545,324.53791",
  "L 675.0593,309.51896",
  "C 669.67688,270.20354 669.5766,238.6171 674.80071,228.06891",
  "C 677.54985,222.51802 679.59682,221.9262 682.99741,225.69908",
  "C 687.47124,230.66271 691.97643,247.00295 697.27952,277.5",
  "L 698.67063,285.5",
  "L 706.26797,270",
  "C 718.02369,246.0161 725.50365,220.45007 723.43511,211.32379",
  "C 723.08309,209.77071 717.83004,197.50126 711.76165,184.05836",
  "C 689.97961,135.806 678,105.28985 678,98.05597",
  "C 678,96.18341 680.47737,98.04457 688.64698,106.05471",
  "C 707.13374,124.18058 721.60104,145.84054 730.36875,168.51918",
  "C 732.22169,173.31199 734.03041,176.75981 734.38814,176.181",
  "C 735.62851,174.17404 740.00008,145.81023 740.01531,139.67053",
  "C 740.04091,129.34946 743.0176,128.14395 747.34275,136.70301",
  "C 756.19728,154.22532 759.70417,193.21961 754.80253,219.65063",
  "C 753.59392,226.16778 752.62224,233.525 752.64324,236",
  "C 752.66794,238.9155 754.93821,246.838 759.09071,258.5",
  "L 765.5,276.5",
  "L 766.1708,259.5",
  "C 769.39401,177.81477 769.84662,124.65474 767.47517,106.30122",
  "C 765.79171,93.272358 761.25027,73.311073 758.05954,64.916108",
  "C 754.14573,54.618675 756.39538,44.000497 761.87011,46.930487",
  "C 763.09997,47.588688 765.5942,50.461092 767.41284,53.313608",
  "C 772.18374,60.79671 780.61627,79.504143 782.37189,86.5",
  "C 786.65679,103.57464 790.98105,134.79036 792.59659,160.30954",
  "C 793.04784,167.43748 793.61784,170.78216 794.27015,170.12985",
  "C 794.81423,169.58577 796.38794,165.39648 797.76729,160.82031",
  "C 799.96711,153.52211 800.33274,150.10451 800.74363,133",
  "C 801.52843,100.33056 795.77854,63.025643 782.48156,14.516804",
  "C 780.66987,7.9075619 779.3467,2.3361696 779.54119,2.1359325",
  "C 781.1591,0.4701825 797.22162,15.578006 803.98998,25.131575",
  "C 809.71375,33.21071 816.70443,47.400266 819.95957,57.546363",
  "C 825.50878,74.843 830.81839,128.93542 829.62359,156",
  "C 828.81633,174.28591 827.09917,182.91832 822.09751,193.83466",
  "C 810.23252,219.7305 801.90125,252.41631 798.4721,286.5235",
  "C 797.58865,295.31057 796.05897,305.425 795.07282,309",
  "C 794.08667,312.575 785.71681,337.1 776.47312,363.5",
  "C 767.22944,389.9 759.30834,413.70238 758.87068,416.39419",
  "C 757.8572,422.62753 754.39025,427.84666 747.75252,433.13137",
  "C 741.93217,437.76532 724.06063,446.70061 714,450.00674",
  "C 707.39771,452.17639 671.00511,468.14104 636.49322,484.00729",
  "C 617.41825,492.77669 590.00226,506.46343 576,514.20702",
  "C 566.54257,519.43721 553.45229,532.33274 545.75711,544",
  "C 540.40061,552.1214 523,583.47577 523,585.00632",
  "C 523,586.99825 536.32803,587.54548 544.20078,585.87679",
  "C 550.51496,584.53846 554.17541,582.75448 563.5,576.47105",
  "C 574.22677,569.24276 598.98077,557.54514 614,552.60706",
  "C 635.10719,545.66734 646.07921,543.83584 670.01301,543.2571",
  "C 688.6098,542.80742 690.89026,542.93572 694.42519,544.63054",
  "C 700.20044,547.39949 702.45481,551.00437 702.33551,557.27964",
  "C 702.23322,562.66008 701.96509,563.45196 692.70365,585.72553",
  "C 672.73981,633.73811 646.07566,662.9657 611.78278,674.42594",
  "C 599.08833,678.66826 541.97133,682.08179 525.27044,679.59625",
  "L 520.04087,678.81795",
  "L 521.54376,683.65897",
  "C 523.67189,690.514 524.56771,722.83819 523.11902,740.5",
  "C 521.51112,760.10294 518.51697,780.26233 513.85139,802.89835",
  "C 509.36292,824.67509 509.36553,825.28569 514.02528,843.5",
  "C 517.51309,857.13332 521.57064,869.31133 533.6467,902.39007",
  "C 546.81727,938.46692 546.34407,936.10427 543.91265,953.64698",
  "C 539.2912,986.99083 531.90452,1003.3875 513.72512,1020.6562",
  "C 495.39938,1038.0638 468.0642,1051.7663 428.64608,1063.3044",
  "C 418.55708,1066.2575 416.78871,1066.495 410,1065.8079 Z",
  "M 427.59456,1060.9725",
  "C 472.83851,1047.7926 506.01222,1029.2803 521.88145,1008.3564",
  "C 532.70669,994.08305 537.35121,981.07207 541.14037,954.4052",
  "C 543.76168,935.95728 544.31145,938.75338 530.89038,902.27414",
  "C 519.63198,871.67312 514.93063,857.54563 511.50117,844.00982",
  "C 507.35614,827.64965 507.27166,822.86726 510.82091,805.5",
  "C 517.0626,774.95805 519.57858,756.93412 520.94679,732.96059",
  "C 521.91789,715.94495 520.88099,687.57001 519.16675,684.24988",
  "C 518.66982,683.28745 517.94731,680.75219 517.56117,678.61597",
  "C 516.95187,675.24522 517.07952,674.81654 518.52717,675.37205",
  "C 526.6718,678.49744 560.82829,678.59365 590.5,675.57479",
  "C 609.79668,673.6115 619.96636,670.14686 634.01543,660.74983",
  "C 656.31227,645.83608 677.88234,616.832 689.97805,585.5",
  "C 691.25201,582.2 693.85444,576.14216 695.76121,572.03814",
  "C 697.85498,567.53164 699.42811,562.37026 699.73314,559.0064",
  "C 700.18446,554.02916 699.95176,553.09612 697.5461,550.23715",
  "C 695.96455,548.3576 693.23281,546.60123 690.92407,545.97956",
  "C 685.55924,544.53496 655.85022,545.50005 644.5,547.48764",
  "C 616.70349,552.35521 587.09359,564.07494 563.54547,579.52979",
  "C 550.62284,588.01104 545.92825,589.36376 531.35293,588.80585",
  "C 525.13053,588.56768 519.75541,588.08875 519.40823,587.74156",
  "C 518.45422,586.78755 535.50131,555.06547 543.42083,543.05772",
  "C 547.39747,537.02825 553.84011,529.06877 558.442,524.5",
  "C 565.11744,517.87261 568.90207,515.17705 580.5,508.78945",
  "C 606.33677,494.55976 696.86167,452.81957 714.5,447.00329",
  "C 739.72316,438.68591 757,425.21796 757,413.87294",
  "C 757,412.66773 765.2921,387.79078 775.42689,358.59082",
  "C 793.58011,306.28847 793.8703,305.32178 794.9666,293.5",
  "C 798.30495,257.50154 806.87711,221.89314 818.59124,195.36422",
  "C 825.67764,179.31569 827,171.55735 827,146.02962",
  "C 827,113.70974 822.22819,71.478923 816.85405,56.237324",
  "C 813.01024,45.335886 810.23066,39.775251 803.24764,29.0173",
  "C 798.32172,21.428486 784.54568,6.1209829 783.54373,7.1229317",
  "C 783.32756,7.3391073 784.46613,12.355516 786.07389,18.270506",
  "C 792.88097,43.313953 799.34736,75.694156 802.19572,99",
  "C 802.90152,104.775 803.46581,118.05 803.44969,128.5",
  "C 803.41649,150.00291 801.82734,160.23679 796.90545,170.64302",
  "C 792.05244,180.90361 791.46821,180.07128 790.11341,160.96668",
  "C 787.95263,130.49651 784.79495,107.3291 780.41243,89.791937",
  "C 776.66126,74.781203 763.73271,49 759.95642,49",
  "C 759.48023,49 758.80585,50.517999 758.45779,52.37333",
  "C 758.00814,54.770146 758.7313,58.576108 760.95591,65.520875",
  "C 764.82272,77.592265 767.31646,88.20141 769.64809,102.5",
  "C 772.40286,119.39354 771.93788,179.52737 768.37031,267.75",
  "C 767.89342,279.54298 767.37741,284 766.48897,284",
  "C 765.78652,284 761.76854,273.82189 757.06168,260.11939",
  "C 749.24738,237.37055 748.9123,236.03941 749.99144,232.03179",
  "C 755.92832,209.98389 756.2931,181.21231 750.94812,156.57357",
  "C 749.1931,148.48346 745.93326,138.29928 744.22369,135.56556",
  "C 743.85719,134.9795 742.7873,140.74192 741.84615,148.37093",
  "C 740.16039,162.03575 735.10825,187.44158 734.24776,186.5811",
  "C 734.00511,186.33845 732.30048,181.65083 730.45969,176.16417",
  "C 724.49065,158.37287 712.46805,137.0419 699.93157,122",
  "C 691.92812,112.39704 683.16788,103.49879 682.43693,104.22974",
  "C 680.66828,105.99839 697.65367,147.48745 720.00717,196",
  "L 726.22766,209.5",
  "L 725.68244,218.5",
  "C 724.77051,233.55315 718.86903,250.94444 706.02462,276.43038",
  "C 698.32176,291.71444 696,294.65115 696,289.1102",
  "C 696,283.68268 690.16599,251.77034 687.4847,242.53106",
  "C 684.90721,233.64944 681.36926,226 679.83891,226",
  "C 678.42914,226 676.25858,230.3645 674.86571,236",
  "C 672.05142,247.38653 674.36713,295.23087 678.90934,319.5441",
  "C 679.69127,323.72961 679.21096,326.15213 674.48282,341.86953",
  "C 655.79028,404.0078 635.6206,444.25116 620.23882,450.0993",
  "C 618.77768,450.65482 611.26371,455.32843 603.5411,460.4851",
  "C 570.2331,482.72606 544.45378,493.69599 527.50666,492.8402",
  "C 521.9712,492.56068 519.39431,491.93901 517.30073,490.37805",
  "L 514.5,488.28984",
  "L 513.83982,465.39492",
  "C 512.55504,420.83945 506.69996,389.22632 496.42271,371.3554",
  "C 493.68891,366.60163 488.62492,361.98999 486.17327,362.02151",
  "C 485.25297,362.03331 482.16087,364.00185 479.30193,366.39598",
  "L 474.10385,370.74893",
  "L 476.62993,373.12206",
  "C 480.58493,376.83759 485.61927,385.81648 488.34129,394.0096",
  "C 493.19192,408.60972 493.82706,443.1486 489.97775,483",
  "C 488.69487,496.28147 488.21753,498.58852 485.74039,503.47983",
  "C 480.87955,513.07793 472.94211,520.04598 458.5,527.39341",
  "C 452.31099,530.54207 446.06434,534.94142 438.5,541.47887",
  "C 432.45,546.70756 425.03355,552.21749 422.01901,553.72315",
  "C 411.83516,558.80965 397.67343,557.68692 382.68095,550.60447",
  "C 376.16512,547.5264 373.35855,545.0489 352.37979,523.8561",
  "L 329.25958,500.5",
  "L 327.64969,491.46545",
  "C 324.3348,472.86258 323.69654,466.05087 323.67069,449",
  "C 323.64309,430.77011 324.74158,420.6391 328.11159,408.04481",
  "C 330.04656,400.81351 329.9807,393 327.98479,393",
  "C 327.52311,393 325.72112,396.7125 323.98036,401.25",
  "C 322.2396,405.7875 319.45736,412.425 317.79759,416",
  "C 313.39498,425.48289 308.43092,438.53409 305.38121,448.64441",
  "C 303.07124,456.30234 302.69577,459.39341 302.60498,471.5",
  "C 302.51588,483.38647 302.19807,486.09412 300.5,489.43547",
  "C 296.26973,497.75951 287.37012,500.24257 278.61965,495.54025",
  "C 275.93546,494.09782 270.76046,492.29023 267.11965,491.52338",
  "C 239.36807,485.67815 218.23066,473.9343 199.08857,453.7256",
  "C 183.80966,437.59533 175.69684,421.22678 172.4603,400",
  "C 169.68439,381.79426 171.07318,368.54464 178.16259,345.59779",
  "C 185.39773,322.17926 189.02517,306.71687 189.68483,296.48285",
  "C 190.16587,289.02015 189.99557,287.24121 188.69695,286.16346",
  "C 187.52171,285.18809 186.61765,285.13426 185.0953,285.949",
  "C 180.47092,288.42389 175.77347,300.45521 168.54209,328.3459",
  "C 166.56764,335.96114 164.29051,343.47133 163.4818,345.0352",
  "C 161.70854,348.46431 158.27204,351.04162 156.42044,350.33109",
  "C 154.79131,349.70594 144.29528,331.0592 140.47319,322",
  "C 132.93372,304.12977 127.46209,283.51881 124.41214,261.5",
  "C 121.94872,243.71552 121.34666,177.66253 123.51191,162.73525",
  "C 125.6531,147.97392 129.12308,136.41329 135.13847,124",
  "C 137.937,118.225 140.59067,112.27507 141.03553,110.77793",
  "C 141.84268,108.06155 141.8391,108.05932 139.32075,109.70941",
  "C 134.95704,112.56862 127.1369,123.13406 122.08192,133",
  "C 114.53229,147.73481 109.13924,166.66667 105.98696,189.5",
  "C 104.24398,202.12515 103.531,259.62443 104.94421,273.59428",
  "C 107.27439,296.62862 111.56633,311.82047 118.90212,323",
  "C 124.61305,331.7033 131.25204,345.65368 134.98371,356.79198",
  "C 136.66347,361.80574 140.42891,378.52863 143.35134,393.95396",
  "C 147.71057,416.96305 148.3995,422 147.18736,422",
  "C 144.18387,422 130.82955,414.77823 123.28227,409.07257",
  "C 108.14777,397.63106 88,375.29307 88,369.95485",
  "C 88,366.26092 81.812961,342.18082 74.105163,315.87581",
  "C 64.633528,283.55121 60.578144,258.79037 60.53159,233",
  "L 60.5,215.5",
  "L 63.641874,209",
  "C 67.660391,200.68637 79.326944,159.32537 87.476495,124.5",
  "C 88.892267,118.45 90.28802,112.6 90.57817,111.5",
  "C 92.48955,104.25367 76.879697,130.10131 65.364607,153.25",
  "C 61.055531,161.9125 56.96069,169 56.264961,169",
  "C 54.700447,169 54.66022,164.78158 56.021629,143.48293",
  "C 59.123978,94.948052 67.619016,58.058859 80.485508,37.25",
  "C 81.930826,34.9125 82.810871,33 82.441163,33",
  "C 80.502169,33 67.690075,42.644098 62.753532,47.819548",
  "C 56.429754,54.449368 53.847926,59.586547 47.857092,77.459656",
  "C 36.228329,112.15302 31.930451,142.25425 31.339509,193.14459",
  "L 30.925602,228.78917",
  "L 26.460289,236.62478",
  "C 14.604693,257.42865 7.398519,282.83324 4.923616,312.55002",
  "C 3.687831,327.38838 4.103797,331.26023 8.248679,343.5",
  "C 13.562428,359.19142 24.874664,376.6203 39.266879,391.29006",
  "C 51.510238,403.76954 57.260532,408.5968 70.5,417.50968",
  "C 76,421.2123 82.3,425.70006 84.5,427.48247",
  "C 99.93967,439.99145 156.05285,475.07037 226.21488,516.07491",
  "C 264.94518,538.70993 276.57258,547.68091 279.42294,557.12699",
  "C 280.19085,559.67183 283.26674,565.814 286.25826,570.77626",
  "C 291.25712,579.06825 301,599.71739 301,602.02002",
  "C 301,603.93252 298.93674,602.96382 298.05161,600.63575",
  "C 295.15487,593.01674 271.22435,581.28013 242.92987,573.60156",
  "C 237.74344,572.19406 227.01207,570.36361 219.08237,569.5339",
  "C 211.15267,568.70418 188.31388,565.92092 168.3295,563.34887",
  "C 148.34512,560.77682 131.70529,558.96137 131.35212,559.31455",
  "C 130.023,560.64367 146.10796,597.494 156.54015,617.01987",
  "C 169.59362,641.45195 187.66666,664.7452 200.23157,673.3311",
  "C 209.68214,679.78889 234.54291,686.80106 249.09946,687.11466",
  "C 263.64975,687.42812 292.92357,682.62662 301.91812,678.45133",
  "C 307.98694,675.63417 309.63092,676.21501 307.96332,680.58716",
  "C 307.35227,682.18922 306.37435,690.6793 305.79016,699.45401",
  "C 304.37732,720.67532 305.95492,740.11405 313.86908,799",
  "L 316.75865,820.5",
  "L 310.48594,834.5",
  "C 304.48806,847.88661 295.24954,867.77831 284.01281,891.5",
  "C 261.67198,938.66336 262.7424,936.10398 263.38797,940.81392",
  "C 263.72045,943.23957 265.52394,949.78625 267.39574,955.3621",
  "C 281.18265,996.43147 307.49115,1025.9296 347.38748,1045.0521",
  "C 362.40125,1052.2482 373.25022,1056.0586 388.39803,1059.456",
  "C 411.75599,1064.6948 414.45768,1064.7993 427.59456,1060.9725 Z",
  "M 685,415.57198",
  "C 685,412.34723 691.21477,382.1556 695.10118,366.5",
  "C 705.2044,325.80123 717.7227,292.99424 729.43496,276.52069",
  "L 732.85319,271.71288",
  "L 734.26704,275.10644",
  "C 743.82049,298.0369 746.01114,327.27397 739.92175,350.57604",
  "C 737.41299,360.17624 731.76784,372.50027 725.93132,381.11883",
  "C 718.19108,392.54852 702.25892,407.06483 688.75,414.99592",
  "C 685.52416,416.88982 685,416.97033 685,415.57198 Z",
  "M 695.76595,407.34747",
  "C 711.569,396.07065 723.20269,382.6083 731.06488,366.5",
  "C 738.41641,351.43798 740.4341,341.97713 740.44863,322.5",
  "C 740.45563,313.06133 739.92713,304.0395 739.15977,300.5",
  "C 737.22149,291.55967 733.03753,277.99975 732.21786,278.00179",
  "C 731.19042,278.00479 726.88182,285.14752 721.97559,294.98232",
  "C 713.09044,312.79307 704.36066,338.80018 697.00549,369.37135",
  "C 693.81753,382.62185 688,409.87961 688,411.56614",
  "C 688,412.60866 689.64639,411.7143 695.76595,407.34747 Z",
  "M 49.5,348.21775",
  "C 43.950218,344.12939 41.545523,341.30231 38.682375,335.5",
  "C 35.60651,329.26661 33.996088,321.74805 34.013137,313.70078",
  "C 34.028376,306.50825 36.97629,291.29764 39.478632,285.5",
  "L 41.205088,281.5",
  "L 42.663866,285.5",
  "C 46.404666,295.75735 54.601034,342.31177 53.800029,348.75211",
  "L 53.5,351.16443",
  "L 49.5,348.21775 Z",
  "M 49.52624,331.79072",
  "C 46.775219,312.43471 42.198538,290 41.000934,290",
  "C 40.090375,290 37.889334,299.91994 37.007085,308",
  "C 36.117533,316.14694 37.37104,326.7897 39.89115,332.48683",
  "C 42.335899,338.01359 50.362151,346.96993 50.815482,344.67708",
  "C 50.988996,343.79948 50.408838,338.00062 49.526241,331.79072 Z"
};

float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t numVerts = 5;
const float lineWidthScale = 0.85;
// const float lineWidthScale = cosf(DEMOLOOP_M_PI / numVerts);


Vertex lines[numVerts * 2];
Triangle polygon[numVerts * 2];

const glm::vec3 twoDAxis(0, 0 , 1);
const RGB white(255, 255, 255);
const RGB pink(255, 132, 242); // 1.0, 0.52, 0.95
const RGB black(0, 0, 0);

const Vertex quad[6] = {
  {0, 0, 0, 0, 0}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0},
  {1, 1, 0, 1, 1}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0}
};

void setColor(const RGB &c) {
  glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, c.r / 255.0, c.g / 255.0, c.b / 255.0, 1.0);
}

void line(float x1, float y1, float x2, float y2) {
  const Vertex v[2] = {{x1, y1, 1}, {x2, y2, 1}};
  gl.bindTexture(gl.getDefaultTexture());
  gl.lines(v, 2);
}

void line(const glm::vec2 &a, const glm::vec2 &b) {
  line(a.x, a.y, b.x, b.y);
}

void lineSegment(const glm::vec2 &start, const vector<glm::vec2> &deltas) {
  glm::vec2 current(start);
  for (const glm::vec2 &delta : deltas) {
    line(current, current + delta);
    current += delta;
  }
  line(current, start);
}

#include <iostream>
#include <regex>

void show_matches(const std::string& in, const std::string& re)
{
    std::smatch m;
    std::regex_search(in, m, std::regex(re, std::regex_constants::ECMAScript));
    if(m.empty()) {
        std::cout << "input=[" << in << "], regex=[" << re << "]: NO MATCH\n";
    } else {
        std::cout << "input=[" << in << "], regex=[" << re << "]: ";
        std::cout << "prefix=[" << m.prefix() << "] ";
        for(std::size_t n = 0; n < m.size(); ++n)
            std::cout << " m[" << n << "]=[" << m[n] << "] ";
        std::cout << "suffix=[" << m.suffix() << "]\n";
    }
}

class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(500, 500, 0, 0, 0) {
    glDisable(GL_DEPTH_TEST);
    // gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
    // glLineWidth(2.0);

    printf("%s\n", svg[0].c_str());



    show_matches(svg[357], "C (\\d+\\.\\d+),(\\d+\\.\\d+) (\\d+\\.\\d+),(\\d+\\.\\d+) (\\d+\\.\\d+),(\\d+\\.\\d+)");
  }

  void Update(float dt) {
    t += dt;
    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;
  }

private:
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}