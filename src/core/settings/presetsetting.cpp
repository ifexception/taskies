#include "presetsetting.h"

namespace tks::Core::Settings
{
PresetSetting::PresetSetting()
    : Uuid()
    , Name()
    , IsDefault(false)
    , Delimiter(DelimiterType::None)
    , TextQualifier(TextQualifierType::None)
    , EmptyValuesHandler(EmptyValues::None)
    , NewLinesHandler(NewLines::None)
    , BooleanHandler(BooleanHandler::None)
    , ExcludeHeaders(false)
    , IncludeAttributes(false)
    , Columns()
{
}
} // namespace tks::Core::Settings
