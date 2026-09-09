#pragma once

#include <Arduino.h>

#include "domainCategories.h"

DomainCategoryMask profileCategoryBlockMask(const char* profileId);
bool profileBlocksCategories(const char* profileId, DomainCategoryMask categories);
