#include "projectionkeyvaluepairmodel.h"

ProjectionKeyValuePairModel::ProjectionKeyValuePairModel(const std::string& key,
    const std::string& value)
    : Key(key)
    , Value(value)
{
}

ProjectionListModel::ProjectionListModel()
    : TaskId(-1)
    , ProjectionKeyValuePairModels()
{
}
