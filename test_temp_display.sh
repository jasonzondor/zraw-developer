#!/bin/bash
timeout 5 ./build/zraw-developer ./tmp/DSCF3710.raf 2>&1 | grep -E "(Camera WB|AdjustmentPanel::set|Label text)" | head -20
