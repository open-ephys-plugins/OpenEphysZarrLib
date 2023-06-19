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

#ifndef ZARRFILEFORMAT_H_INCLUDED
#define ZARRFILEFORMAT_H_INCLUDED

#include <CommonLibHeader.h>

 // handles for z5 filesystem objects
#include "z5/filesystem/handle.hxx"

 // handles for z5 filesystem objects
#include "z5/dataset.hxx"

#define PROCESS_ERROR std::cerr << error.getCDetailMsg() << std::endl; return -1
#define CHECK_ERROR(x) if (x) std::cerr << "Error at " << __FILE__ " " << __LINE__ << std::endl;

#ifndef CHUNK_XSIZE
#define CHUNK_XSIZE 2048
#endif

#define DEFAULT_STR_SIZE 256

namespace OpenEphysZarr
{

class ZarrDataset;

/** 

	Base class for all HDF5 files

*/
class COMMON_LIB ZarrFile
{
public:

	/** Constructor */
	ZarrFile(const String& filename, bool readOnly = false);

	/** Destructor -- closes the file if it's open  */
    virtual ~ZarrFile();

	/** Creates the file (if path is valid) or opens for reading (if the file exists) */
    int open();

	/** Closes the file (called automatically in destructor) */
    void close();

	class COMMON_LIB BaseDataType {
	public:
		enum Type { T_U8, T_U16, T_U32, T_U64, T_I8, T_I16, T_I32, T_I64, T_F32, T_F64, T_STR };
		BaseDataType();
		BaseDataType(Type, size_t);
		Type type;
		size_t typeSize;
		String typeString;

		//handy accessors
		static const BaseDataType U8;
		static const BaseDataType U16;
		static const BaseDataType U32;
		static const BaseDataType U64;
		static const BaseDataType I8;
		static const BaseDataType I16;
		static const BaseDataType I32;
		static const BaseDataType I64;
		static const BaseDataType F32;
		static const BaseDataType F64;
		static const BaseDataType DSTR;
		static BaseDataType STR(size_t size);
	};

protected:

	/** Creates the basic file structure upon opening */
    virtual int createFileStructure() = 0;

	/** Sets an attribute (of any data type) at a given location in the file */
	int setAttribute(BaseDataType type, const void* data, String path, String name);

	/** Sets a string attribute at a given location in the file */
    int setAttributeStr(const String& value, String path, String name);

	/** Sets an object reference attribute for a given location in the file */
	int setAttributeRef(String referencePath, String attributePath, String attributeName);

	/** Sets an array attribute (of any data type) at a given location in the file */
	int setAttributeArray(BaseDataType type, const void* data, int size, String path, String name);

	/** Sets a string array attribute at a given location in the file */
	int setAttributeStrArray(const StringArray& data, String path, String name);

	/** Creates a new group (throws an exception if it exists) */
    int createGroup(String path);

	/** Creates a new group (ignores if it exists) */
	int createGroupIfDoesNotExist(String path);

	/** Returns a pointer to a dataset at a given path*/
    ZarrDataset* getDataSet(String path);

	/** Creates a non-modifiable dataset with a string value */
	void createStringDataSet(String path, String value);
    
    /** Creates a dataset that holds an array of references to groups within the file */
    void createReferenceDataSet(String path, StringArray references);

	/** Creates a reference to another location in the file */
	void createReference(String path, String reference);

    /** aliases for createDataSet */
	ZarrDataset* createDataSet(BaseDataType type, int sizeX, int chunkX, String path);
	ZarrDataset* createDataSet(BaseDataType type, int sizeX, int sizeY, int chunkX, String path);
	ZarrDataset* createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, String path);
	ZarrDataset* createDataSet(BaseDataType type, int sizeX, int sizeY, int sizeZ, int chunkX, int chunkY, String path);

private:
	int setAttributeStrArray(Array<const char*>& data, int maxSize, String path, String name);

    /** Create an extendable dataset */
	ZarrDataset* createDataSet(BaseDataType type, int dimension, int* size, int* chunking, String path);

    z5::filesystem::handle::File file;

	bool readOnly;

	const String filename;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZarrFile);
};

/** 

	Represents an Zarr Dataset that can be extended indefinitely
	in blocks.

*/
class COMMON_LIB ZarrDataset
{
public:

	/** Constructor */
	ZarrDataset(z5::Dataset* dataset); //

	/** Destructor */
    ~ZarrDataset();

	/** Writes a 1D block of data (samples) */
	int writeDataBlock(int xDataSize, ZarrFile::BaseDataType type, const void* data);

	/** Writes a 2D block of data (samples x channels) */
	int writeDataBlock(int xDataSize, int yDataSize, ZarrFile::BaseDataType type, const void* data);

	/** Writes a row of data in a 2D block (samples x channels) */
	int writeDataRow(int yPos, int xDataSize, ZarrFile::BaseDataType type, const void* data);

	/** Returns the number of samples written for each row (channel) */
    void getRowXPositions(Array<uint32>& rows);

private:
    int xPos;
    int xChunkSize;
    int size[3];
    int dimension;
    Array<uint32> rowXPos;
    
	std::unique_ptr<z5::Dataset> dataset;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZarrDataset);
};

}


#endif  // HDF5FILEFORMAT_H_INCLUDED
