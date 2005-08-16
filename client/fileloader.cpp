//////////////////////////////////////////////////////////////////////
// OTItemEditor
//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#include "fileloader.h"

FileLoader::FileLoader()
{
	m_file = NULL;
	m_buffer = new unsigned char[1024];
	m_buffer_size = 1024;
	m_lastError = ERROR_NONE;
}


FileLoader::~FileLoader()
{
	if(m_file){
		fclose(m_file);
		m_file = NULL;
	}

	delete[] m_buffer;
}


bool FileLoader::openFile(const char* filename)
{
	unsigned long version;
	m_file = fopen(filename, "rb");
	if(m_file){
		fread(&version, sizeof(unsigned long), 1, m_file);
		if(version > 0){
			fclose(m_file);
			m_lastError = ERROR_INVALID_FILE_VERSION;
			return false;
		}
		else{
			return true;
		}
	}
	else{
		m_lastError = ERROR_CAN_NOT_OPEN;
		return false;
	}
}

const unsigned char* FileLoader::getProps(const NODE node, unsigned long &size)
{
	if(!checks(node))
		return NULL;
	
	if(!safeSeek(node)){
		return NULL;
	}
	
	int byte, position;
	if(!readByte(byte))
		return NULL;

	if(byte != NODE_START){
		m_lastError = ERROR_INVALID_FORMAT;
		return NULL;
	}
	//read node type
	if(!readByte(byte))
		return NO_NODE;
	
	position = 0;
	while(1){
		if(!readByte(byte))
			return NULL;
		
		if(byte == NODE_END || byte ==NODE_START)
			break;
		else if(byte == ESCAPE_CHAR){
			if(!readByte(byte))
				return NULL;
		}
		if(position >= m_buffer_size){
			unsigned char *tmp = new unsigned char[m_buffer_size+1024];
			memcpy(tmp, m_buffer, m_buffer_size);
			m_buffer_size = m_buffer_size + 1024;
			delete m_buffer;
			m_buffer = tmp;
		}
		m_buffer[position] = byte;
		position++;
	}
	size = position;

	return m_buffer;
}

const NODE FileLoader::getChildNode(const NODE parent, unsigned long &type)
{
	if(!checks(1))
		return NO_NODE;

	int seek_pos, byte;
	long tmp;
	if(!parent){
		seek_pos = 4;
	}
	else{
		seek_pos = parent;
	}

	if(!safeSeek(seek_pos)){
		return NO_NODE;
	}

	if(!readByte(byte))
		return NO_NODE;

	if(byte != NODE_START){
		m_lastError = ERROR_INVALID_FORMAT;
		return NO_NODE;
	}

	if(!parent){
		if(safeTell(tmp)){
			if(!readByte(byte))
				return NO_NODE;
					
			type = byte;
			return (NODE)tmp;
		}
		else{
			return NO_NODE;
		}
	}
	else{
		if(!readByte(byte))
			return NO_NODE;
		
		while(1){
			if(!readByte(byte))
				return NO_NODE;
		
			if(byte == NODE_END){
				return NO_NODE;
			}
			else if(byte == NODE_START){
				if(safeTell(tmp)){
					if(!readByte(byte))
						return NO_NODE;
					
					type = byte;
					return (NODE)tmp;
				}
				else{
					return NO_NODE;
				}
			}
			else if(byte == ESCAPE_CHAR){
				if(!readByte(byte))
					return NO_NODE;
			}
		}
	}
	return NO_NODE;

}

const NODE FileLoader::getNextNode(const NODE prev, unsigned long &type)
{
	if(!checks(prev))
		return NO_NODE;

	if(!safeSeek(prev)){
		return NO_NODE;
	}

	int byte;
	long tmp;
	if(!readByte(byte))
		return NO_NODE;

	if(byte != NODE_START){
		m_lastError = ERROR_INVALID_FORMAT;
		return NO_NODE;
	}
	if(!readByte(byte))
		return NO_NODE;

	int level;
	level = 1;
	while(1){
		if(!readByte(byte))
			return NO_NODE;
		
		if(byte == NODE_END){
			level--;
			if(level == 0){
				if(!readByte(byte))
					return NO_NODE;
				
				if(byte != NODE_START){
					m_lastError = ERROR_INVALID_FORMAT;
					return NO_NODE;
				}
				else{
					if(safeTell(tmp)){
						if(!readByte(byte))
							return NO_NODE;

						type = byte;
						return (NODE)tmp;
					}
					else{
						return NO_NODE;
					}
				}
			}
		}
		else if(byte == NODE_START){
			level++;
		}
		else if(byte == ESCAPE_CHAR){
			if(!readByte(byte))
				return NO_NODE;
		}
	}
	return NO_NODE;
}


inline bool FileLoader::readByte(int &value)
{
	value = fgetc(m_file);
	if(value == EOF){
		m_lastError = ERROR_EOF;
		return false;
	}
	else
		return true;
}

inline bool FileLoader::checks(const NODE node)
{
	if(!m_file){
		m_lastError = ERROR_NOT_OPEN;
		return false;
	}
	if(!node){
		m_lastError = ERROR_INVALID_NODE;
		return false;
	}

	return true;
}

inline bool FileLoader::safeSeek(unsigned long pos){

	if(fseek(m_file, pos, SEEK_SET)){
		m_lastError = ERROR_SEEK_ERROR;
		return false;
	}
	return true;
}



inline bool FileLoader::safeTell(long &pos){

	pos = ftell(m_file);
	if(pos == -1){
		m_lastError = ERROR_TELL_ERROR;
		return false;
	}
	else{
		pos = pos - 1;
		return true;
	}
}