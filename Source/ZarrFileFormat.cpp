/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2023 Open Ephys

 ------------------------------------------------------------------

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "ZarrFileFormat.h"

 // factory functions to create files, groups and datasets
#include "z5/factory.hxx"

// attribute functionality
#include "z5/attributes.hxx"

using namespace OpenEphysZarr;


ZarrFile::ZarrFile(const String& filename_, bool readOnly_) 
    : file(filename_.toStdString()), filename(filename_), readOnly(readOnly_)
{
    
}

ZarrFile::~ZarrFile()
{
    close();
}

int ZarrFile::open()
{

    File f(filename);

    if (!f.getParentDirectory().exists())
    {
        return -1;
    }
    
    if (!file.exists() && !readOnly)
    {
        // create a new file
        z5::createFile(file, true);

        return 0;
    }

    if (readOnly && !file.exists())
        return -1;

    return 0;
}

void ZarrFile::close()
{
    // closes automatically when handle is destroyed
}

int ZarrFile::setAttribute(BaseDataType type, const void* data, String path, String name)
{
    return setAttributeArray(type, data, 1, path, name);
}

int ZarrFile::setAttributeRef(String referencePath, String attributePath, String attributeName)
{

    const auto dsHandle = z5::filesystem::handle::Dataset(file, attributePath.toStdString());

    nlohmann::json attributesIn;
    attributesIn[attributeName.toStdString()] = referencePath.toStdString();
    z5::writeAttributes(dsHandle, attributesIn);
    
    return 0;
}


int ZarrFile::setAttributeArray(BaseDataType type, const void* data, int size, String path, String name)
{
    
    const auto dsHandle = z5::filesystem::handle::Dataset(file, path.toStdString());

    nlohmann::json attributesIn;
    
    // TODO : convert array to JSON
    attributesIn[name.toStdString()] = "attributeArray";
    z5::writeAttributes(dsHandle, attributesIn);
    
    return 0;
}

int ZarrFile::setAttributeStrArray(const StringArray& data, String path, String name)
{

   Array<const char*> array;
    for (auto i = 0; i < data.size(); ++i) {
        array.add(data[i].getCharPointer());
    }
    
    return setAttributeStrArray(array, path, name);
}


int ZarrFile::setAttributeStr(const String& value, String path, String name)
{
    const auto dsHandle = z5::filesystem::handle::Dataset(file, path.toStdString());

    nlohmann::json attributesIn;
    attributesIn[name.toStdString()] = value.toStdString();
    z5::writeAttributes(dsHandle, attributesIn);

    return 0;
}


int ZarrFile::setAttributeStrArray(Array<const char*>& data, int maxSize, String path, String name)
{

    const auto dsHandle = z5::filesystem::handle::Dataset(file, path.toStdString());

    std::vector<const char*> vector;
    for (auto i = 0; i < data.size(); ++i) {
        vector.push_back(data[i]);
    }
    
    nlohmann::json attributesIn;
    attributesIn[name.toStdString()] = nlohmann::basic_json(vector);
    z5::writeAttributes(dsHandle, attributesIn);
	
    return 0;
}

int ZarrFile::createGroup(String path)
{
    
    int slashIndex = path.lastIndexOf("/");
    String root = path.substring(0, slashIndex);
    String name = path.substring(slashIndex + 1);
    
    const auto grpHandle = z5::filesystem::handle::Group(file, root.toStdString());

    z5::createGroup(grpHandle, name.toStdString());
    
    return 0;
}

int ZarrFile::createGroupIfDoesNotExist(String path)
{

    const auto grpHandle = z5::filesystem::handle::Group(file, path.toStdString());

    if (!grpHandle.exists())
    {
        createGroup(path);
    }
    
	return 0;
}

ZarrDataset* ZarrFile::getDataSet(String path)
{
    return nullptr;
}


void ZarrFile::createReference(String path, String reference)
{

    
}

void ZarrFile::createReferenceDataSet(String path, StringArray references)
{

   

}

void ZarrFile::createStringDataSet(String path, String value)
{

    

}

ZarrDataset* ZarrFile::createDataSet(BaseDataType type, int sizeX, int chunkX, String path)
{
    int chunks[3] = {chunkX, 0, 0};
    return createDataSet(type,1,&sizeX,chunks,path);
}

ZarrDataset* ZarrFile::createDataSet(BaseDataType type, int sizeX, int sizeY, int chunkX, String path)
{
    int size[2];
    int chunks[3] = {chunkX, 0, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    return createDataSet(type,2,size,chunks,path);
}

ZarrDataset* ZarrFile::createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, String path)
{
    int size[3];
    int chunks[3] = {chunkX, 0, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    size[2] = sizeZ;
    return createDataSet(type,3,size,chunks,path);
}

ZarrDataset* ZarrFile::createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, int chunkY, String path)
{
    int size[3];
    int chunks[3] = {chunkX, chunkY, 0};
    size[0] = sizeX;
    size[1] = sizeY;
    size[2] = sizeZ;
    return createDataSet(type,3,size,chunks,path);
}

ZarrDataset* ZarrFile::createDataSet(BaseDataType type, int dimension, int* size, int* chunking, String path)
{

    int slashIndex = path.lastIndexOf("/");
    String root = path.substring(0, slashIndex);
    String name = path.substring(slashIndex + 1);

    const auto grpHandle = z5::filesystem::handle::Group(file, root.toStdString());


    const std::string dsName = "data";
    std::vector<size_t> shape;
    std::vector<size_t> chunks; 
    
    for (int i = 0; i < dimension; i++)
    {
        shape.push_back(*(size + i));
        chunks.push_back(*(size + i));
    }
    
    std::unique_ptr<z5::Dataset> ds = z5::createDataset(grpHandle, 
                                name.toStdString(), 
                                type.typeString.toStdString(), 
                                shape, 
                                chunks);
    
    return new ZarrDataset(ds.release());


}



//BaseDataType

ZarrFile::BaseDataType::BaseDataType(ZarrFile::BaseDataType::Type t, size_t s)
	: type(t), typeSize(s)
{

    switch (t)
    {
    case BaseDataType::Type::T_I8:
        typeString = "int8";
        break;
    case BaseDataType::Type::T_I16:
        typeString = "int16";
        break;
    case BaseDataType::Type::T_I32:
        typeString = "int32";
        break;
    case BaseDataType::Type::T_I64:
        typeString = "int64";
        break;
    case BaseDataType::Type::T_U8:
        typeString = "uint8";
        break;
    case BaseDataType::Type::T_U16:
        typeString = "uint16";
        break;
    case BaseDataType::Type::T_U32:
        typeString = "uint32";
        break;
    case BaseDataType::Type::T_U64:
        typeString = "uint64";
        break;
    case BaseDataType::Type::T_F32:
        typeString = "float32";
        break;
    case BaseDataType::Type::T_F64:
        typeString = "float64";
        break;
    default:
        typeString = "int32";
    }
}

ZarrFile::BaseDataType::BaseDataType()
	: type(T_I32), typeSize(1), typeString("int32")
{}

ZarrFile::BaseDataType ZarrFile::BaseDataType::STR(size_t size)
{
	return ZarrFile::BaseDataType(T_STR, size);
}

const ZarrFile::BaseDataType ZarrFile::BaseDataType::U8 = ZarrFile::BaseDataType(T_U8, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::U16 = ZarrFile::BaseDataType(T_U16, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::U32 = ZarrFile::BaseDataType(T_U32, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::U64 = ZarrFile::BaseDataType(T_U64, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::I8 = ZarrFile::BaseDataType(T_I8, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::I16 = ZarrFile::BaseDataType(T_I16, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::I32 = ZarrFile::BaseDataType(T_I32, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::I64 = ZarrFile::BaseDataType(T_I64, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::F32 = ZarrFile::BaseDataType(T_F32, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::F64 = ZarrFile::BaseDataType(T_F64, 1);
const ZarrFile::BaseDataType ZarrFile::BaseDataType::DSTR = ZarrFile::BaseDataType(T_STR, DEFAULT_STR_SIZE);


ZarrDataset::ZarrDataset(z5::Dataset* dataset_)
{
    dataset.reset(dataset_);
}

ZarrDataset::~ZarrDataset()
{

}
int ZarrDataset::writeDataBlock(int xDataSize, ZarrFile::BaseDataType type, const void* data)
{
    return writeDataBlock(xDataSize, size[1], type, data);
}

int ZarrDataset::writeDataBlock(int xDataSize, int yDataSize, ZarrFile::BaseDataType type, const void* data)
{
    
    z5::types::ShapeType chunkIndices;
    // TODO: determine chunk indices
    
    dataset->writeChunk(chunkIndices, data);

    return 0;
}


int ZarrDataset::writeDataRow(int yPos, int xDataSize, ZarrFile::BaseDataType type, const void* data)
{

    z5::types::ShapeType chunkIndices;
    // TODO: determine chunk indices

    dataset->writeChunk(chunkIndices, data);
    
    return 0;
}

void ZarrDataset::getRowXPositions(Array<uint32>& rows)
{
    rows.clear();
    rows.addArray(rowXPos);
}

