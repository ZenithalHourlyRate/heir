#include "lib/Analysis/NoisePropagation/Params.h"

namespace mlir {
namespace heir {

CostModel::BGVAddCostEntries CostModel::AllBGVAddCostEntries = {
    {BGVCostEntry(2048, 1, 2), 14.11},  {BGVCostEntry(2048, 2, 2), 22.24},
    {BGVCostEntry(2048, 3, 2), 47.26},  {BGVCostEntry(2048, 4, 2), 76.32},
    {BGVCostEntry(4096, 1, 2), 36.65},  {BGVCostEntry(4096, 2, 2), 79.63},
    {BGVCostEntry(4096, 3, 2), 119.37}, {BGVCostEntry(4096, 4, 2), 169.43},
    {BGVCostEntry(8192, 1, 2), 102.62}, {BGVCostEntry(8192, 2, 2), 180.56},
    {BGVCostEntry(8192, 3, 2), 252.82}, {BGVCostEntry(8192, 4, 2), 338.86},
};
CostModel::BGVMultCostEntries CostModel::AllBGVMultCostEntries = {
    {BGVMultCostEntry(2048, 1, 2, 2), 74.33},
    {BGVMultCostEntry(2048, 2, 2, 2), 111.09},
    {BGVMultCostEntry(2048, 3, 2, 2), 146.84},
    {BGVMultCostEntry(2048, 4, 2, 2), 188.30},
    {BGVMultCostEntry(4096, 1, 2, 2), 166.81},
    {BGVMultCostEntry(4096, 2, 2, 2), 241.12},
    {BGVMultCostEntry(4096, 3, 2, 2), 321.61},
    {BGVMultCostEntry(4096, 4, 2, 2), 400.17},
    {BGVMultCostEntry(8192, 1, 2, 2), 370.40},
    {BGVMultCostEntry(8192, 2, 2, 2), 521.00},
    {BGVMultCostEntry(8192, 3, 2, 2), 665.82},
    {BGVMultCostEntry(8192, 4, 2, 2), 816.04},
};
CostModel::BGVRelinBVCostEntries CostModel::AllBGVRelinBVCostEntries = {
    {BGVRelinBVCostEntry(2048, 1, 2, 0), 152.74},
    {BGVRelinBVCostEntry(2048, 2, 2, 0), 258.62},
    {BGVRelinBVCostEntry(2048, 3, 2, 0), 370.23},
    {BGVRelinBVCostEntry(2048, 4, 2, 0), 749.37},
    {BGVRelinBVCostEntry(4096, 1, 2, 0), 378.12},
    {BGVRelinBVCostEntry(4096, 2, 2, 0), 695.19},
    {BGVRelinBVCostEntry(4096, 3, 2, 0), 1059.66},
    {BGVRelinBVCostEntry(4096, 4, 2, 0), 1492.71},
    {BGVRelinBVCostEntry(8192, 1, 2, 0), 792.63},
    {BGVRelinBVCostEntry(8192, 2, 2, 0), 1403.47},
    {BGVRelinBVCostEntry(8192, 3, 2, 0), 2108.87},
    {BGVRelinBVCostEntry(8192, 4, 2, 0), 2931.47},
    {BGVRelinBVCostEntry(2048, 1, 2, 30), 500.83},
    {BGVRelinBVCostEntry(2048, 2, 2, 30), 838.17},
    {BGVRelinBVCostEntry(2048, 3, 2, 30), 1171.68},
    {BGVRelinBVCostEntry(2048, 4, 2, 30), 1586.43},
    {BGVRelinBVCostEntry(4096, 1, 2, 30), 1141.94},
    {BGVRelinBVCostEntry(4096, 2, 2, 30), 1689.59},
    {BGVRelinBVCostEntry(4096, 3, 2, 30), 2312.83},
    {BGVRelinBVCostEntry(4096, 4, 2, 30), 3018.11},
    {BGVRelinBVCostEntry(8192, 1, 2, 30), 2319.72},
    {BGVRelinBVCostEntry(8192, 2, 2, 30), 3407.71},
    {BGVRelinBVCostEntry(8192, 3, 2, 30), 4594.63},
    {BGVRelinBVCostEntry(8192, 4, 2, 30), 6082.03},
};
CostModel::BGVRelinHYBRIDCostEntries CostModel::AllBGVRelinHYBRIDCostEntries = {
    {BGVRelinHYBRIDCostEntry(2048, 1, 2, 1, 2), 1141.71},
    {BGVRelinHYBRIDCostEntry(2048, 2, 2, 2, 2), 1720.99},
    {BGVRelinHYBRIDCostEntry(2048, 3, 2, 3, 2), 1872.51},
    {BGVRelinHYBRIDCostEntry(2048, 4, 2, 4, 2), 2322.25},
    {BGVRelinHYBRIDCostEntry(4096, 1, 2, 1, 2), 1842.45},
    {BGVRelinHYBRIDCostEntry(4096, 2, 2, 2, 2), 3030.90},
    {BGVRelinHYBRIDCostEntry(4096, 3, 2, 3, 2), 3302.92},
    {BGVRelinHYBRIDCostEntry(4096, 4, 2, 4, 2), 4008.83},
    {BGVRelinHYBRIDCostEntry(8192, 1, 2, 1, 2), 3280.30},
    {BGVRelinHYBRIDCostEntry(8192, 2, 2, 2, 2), 5418.12},
    {BGVRelinHYBRIDCostEntry(8192, 3, 2, 3, 2), 5867.09},
    {BGVRelinHYBRIDCostEntry(8192, 4, 2, 4, 2), 7654.43},
};
CostModel::BGVModReduceCostEntries CostModel::AllBGVModReduceCostEntries = {
    {BGVCostEntry(2048, 1, 2), 142.44}, {BGVCostEntry(2048, 2, 2), 152.49},
    {BGVCostEntry(2048, 3, 2), 162.74}, {BGVCostEntry(2048, 4, 2), 169.19},
    {BGVCostEntry(4096, 1, 2), 303.88}, {BGVCostEntry(4096, 2, 2), 318.67},
    {BGVCostEntry(4096, 3, 2), 341.04}, {BGVCostEntry(4096, 4, 2), 355.75},
    {BGVCostEntry(8192, 1, 2), 637.92}, {BGVCostEntry(8192, 2, 2), 670.68},
    {BGVCostEntry(8192, 3, 2), 717.73}, {BGVCostEntry(8192, 4, 2), 736.96},
};
}  // namespace heir
}  // namespace mlir
