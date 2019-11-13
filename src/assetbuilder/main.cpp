#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

/* CF defines types Point and Component so it needs to be included early */
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <polar/asset/audio.h>
#include <polar/asset/font.h>
#include <polar/asset/image.h>
#include <polar/asset/level.h>
#include <polar/asset/material.h>
#include <polar/asset/model.h>
#include <polar/asset/shaderprogram.h>
#include <polar/asset/text.h>
#include <polar/core/debugmanager.h>
#include <polar/fs/local.h>
#include <polar/util/debug.h>
#include <polar/util/endian.h>
#include <polar/util/getline.h>
#include <zlib.h>

int main(int argc, char **argv) {
	using namespace polar;

	std::string path = (argc >= 2) ? argv[1] : fs::local::appdir() + "/assets";
	std::string buildPath = (argc >= 3) ? argv[2] : path + "/build";
	auto files            = fs::local::listdir(path);

	std::unordered_map<
	    std::string,
	    std::function<std::string(const std::string &, core::serializer &)>>
	    converters;
	converters["txt"] = [](const std::string &data, core::serializer &s) {
		s << data;
		return asset::name<std::string>();
	};
	converters["ttf"] = [](const std::string &data, core::serializer &s) {
		s << data;
		return asset::name<asset::font>();
	};
	converters["otf"] = [](const std::string &data, core::serializer &s) {
		s << data;
		return asset::name<asset::font>();
	};
	converters["png"] = [](const std::string &data, core::serializer &s) {
		asset::image asset;
		std::istringstream iss(data);

		std::string header(8, ' ');
		iss.read(&header[0], 8);
		if(header != "\x89"
		             "PNG"
		             "\x0D\x0A"
		             "\x1A"
		             "\xA") {
			debugmanager()->fatal("invalid PNG signature");
		}

		int zResult = Z_OK;
		z_stream inflateStream;
		inflateStream.zalloc   = Z_NULL;
		inflateStream.zfree    = Z_NULL;
		inflateStream.opaque   = Z_NULL;
		inflateStream.avail_in = 0;
		inflateStream.next_in  = Z_NULL;

		std::vector<uint8_t> filteredBytes;
		bool atEnd = false;
		size_t numChannels = 0;

		for(size_t numChunks = 0; !atEnd; ++numChunks) {
			uint32_t dataSize;
			iss.read(reinterpret_cast<char *>(&dataSize), sizeof(dataSize));
			dataSize = swapbe(dataSize);
			if(dataSize > (1u << 31)) {
				debugmanager()->fatal("chunk data size exceeds 2^31 bytes");
			}

			std::string chunkType(4, ' ');
			iss.read(&chunkType[0], 4);

			if(numChunks == 0) {
				if(chunkType == "IHDR") {
					iss.read(reinterpret_cast<char *>(&asset.width), sizeof(asset.width));
					asset.width = swapbe(asset.width);
					if(asset.width == 0) {
						debugmanager()->fatal("image width cannot be 0");
					}
					if(asset.width > (1u << 31)) {
						debugmanager()->fatal("image width exceeds 2^31 bytes");
					}

					iss.read(reinterpret_cast<char *>(&asset.height), sizeof(asset.height));
					asset.height = swapbe(asset.height);
					if(asset.height == 0) {
						debugmanager()->fatal("image height cannot be 0");
					}
					if(asset.height > (1u << 31)) {
						debugmanager()->fatal("image height exceeds 2^31 bytes");
					}

					uint8_t bitDepth;
					iss.read(reinterpret_cast<char *>(&bitDepth), sizeof(bitDepth));
					if(bitDepth != 8) {
						debugmanager()->fatal("bit depth must be 8");
					}

					uint8_t colorType;
					iss.read(reinterpret_cast<char *>(&colorType), sizeof(colorType));
					if(colorType & 0x1) {
						debugmanager()->fatal("color type cannot be palette");
					}
					if(!(colorType & 0x2)) {
						debugmanager()->fatal("color type must be true color");
					}
					numChannels = (colorType & 0x4) ? 4 : 3;

					uint8_t compressionMethod;
					iss.read(reinterpret_cast<char *>(&compressionMethod), sizeof(compressionMethod));
					if(compressionMethod != 0) {
						debugmanager()->fatal("compression method must be 0");
					}

					uint8_t filterMethod;
					iss.read(reinterpret_cast<char *>(&filterMethod), sizeof(filterMethod));
					if(filterMethod != 0) {
						debugmanager()->fatal("filter method must be 0");
					}

					uint8_t interlaceMethod;
					iss.read(reinterpret_cast<char *>(&interlaceMethod), sizeof(interlaceMethod));
					if(interlaceMethod != 0) {
						debugmanager()->fatal("interlace method must be 0");
					}

					// ignore CRC
					iss.ignore(4);

					asset.pixels.resize(asset.width * asset.height);

					/* number of bytes in scanline = image width + 1
					 * number of scanlines in image = image height
					 */
					uint32_t scanlineSize = 1 + asset.width * numChannels;
					uint32_t filteredSize = scanlineSize * asset.height;
					filteredBytes.resize(filteredSize);
					inflateStream.avail_out = filteredSize;
					inflateStream.next_out  = &filteredBytes[0];

					zResult = inflateInit(&inflateStream);
					if(zResult != Z_OK) {
						debugmanager()->fatal("inflateInit failed");
					}
				} else {
					debugmanager()->fatal("first chunk must be IHDR");
				}
			} else {
				if(chunkType == "IDAT") {
					std::vector<uint8_t> compressedBytes(dataSize, ' ');
					iss.read(reinterpret_cast<char *>(&compressedBytes[0]), dataSize);

					inflateStream.avail_in = dataSize;
					inflateStream.next_in  = &compressedBytes[0];

					zResult = inflate(&inflateStream, Z_NO_FLUSH);
					if(zResult != Z_OK && zResult != Z_STREAM_END) {
						debugmanager()->fatal("inflate failed (zResult = ", zResult, ')');
					}

					// ignore CRC
					iss.ignore(4);
				} else if(chunkType == "IEND") {
					zResult = inflateEnd(&inflateStream);
					if(zResult != Z_OK) {
						debugmanager()->fatal("inflateEnd failed (zResult = ", zResult, ')');
					}

					size_t pos = 0;
					for(uint32_t scanline = 0; scanline < asset.height; ++scanline) {
						uint8_t filterType = filteredBytes[pos++];
						//debugmanager()->info(int(filterType));

						for(uint32_t column = 0; column < asset.width; ++column) {
							asset::imagepixel &pixel = asset.pixels[scanline * asset.width + column];
							for(size_t component = 0; component < numChannels; ++component) {
								const auto paethPredictor = [](const int a, const int b, const int c) {
									int p = a + b - c;   // initial estimate
									int pa = abs(p - a); // distance to a
									int pb = abs(p - b); // distance to b
									int pc = abs(p - c); // distance to c

									// return nearest of {a, b, c}
									if(pa <= pb && pa <= pc) {
										return a;
									} else if(pb <= pc) {
										return b;
									} else {
										return c;
									}
								};

								uint8_t left, up, upperLeft;
								switch(filterType) {
								case 0:
									pixel[component] = filteredBytes[pos++];
									break;
								case 1:
									if(column > 0) {
										left = asset.pixels[scanline * asset.width + column - 1][component];
									} else {
										left = 0;
									}
									pixel[component] = filteredBytes[pos++] + left;
									break;
								case 2:
									if(scanline > 0) {
										up = asset.pixels[(scanline - 1) * asset.width + column][component];
									} else {
										up = 0;
									}
									pixel[component] = filteredBytes[pos++] + up;
									break;
								case 3:
									if(column > 0) {
										left = asset.pixels[scanline * asset.width + column - 1][component];
									} else {
										left = 0;
									}
									if(scanline > 0) {
										up = asset.pixels[(scanline - 1) * asset.width + column][component];
									} else {
										up = 0;
									}
									pixel[component] = filteredBytes[pos++] + ((left + up) >> 1);
									break;
								case 4:
									if(column > 0) {
										left = asset.pixels[scanline * asset.width + column - 1][component];
									} else {
										left = 0;
									}
									if(scanline > 0) {
										up = asset.pixels[(scanline - 1) * asset.width + column][component];
										if(column > 0) {
											upperLeft = asset.pixels[(scanline - 1) * asset.width + column - 1][component];
										} else {
											upperLeft = 0;
										}
									} else {
										up        = 0;
										upperLeft = 0;
									}
									pixel[component] = filteredBytes[pos++] + paethPredictor(left, up, upperLeft);
									break;
								default:
									debugmanager()->fatal("png: unsupported filter type (", int(filterType), ')');
									break;
								}
							}
						}
					}

					atEnd = true;
				} else if(chunkType == "bKGD") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "pHYs") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "tIME") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "iTXt") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else if(chunkType == "tEXt") {
					iss.ignore(dataSize);
					iss.ignore(4);
				} else {
					std::stringstream ss;
					ss << "unrecognized chunk type `" << chunkType << '`';
					debugmanager()->fatal(ss.str());
				}
			}
		}

		s << asset;
		return asset::name<asset::image>();
	};
	converters["wav"] = [](const std::string &data, core::serializer &s) {
		asset::audio asset;
		std::istringstream iss(data);

		std::string riffHeader(4, ' ');
		iss.read(&riffHeader[0], 4);
		if(riffHeader != "RIFF") {
			debugmanager()->fatal("missing riff header");
		}

		uint32_t riffSize;
		iss.read(reinterpret_cast<char *>(&riffSize), sizeof(riffSize));
		riffSize = swaple(riffSize);

		uint32_t riffSizeAccum = 0;

		if(riffSize - riffSizeAccum < 4) {
			debugmanager()->fatal("wave header does not fit into riff size");
		}
		std::string waveHeader(4, ' ');
		iss.read(&waveHeader[0], 4);
		if(waveHeader != "WAVE") {
			debugmanager()->fatal("missing wave header");
		}
		riffSizeAccum += 4;

		uint16_t bitsPerSample = 0;

		while(riffSize - riffSizeAccum > 0) {
			if(riffSize - riffSizeAccum < 8) {
				debugmanager()->fatal(
				    "chunk header and size do not fit into riff size");
			}

			std::string chunkHeader(4, ' ');
			iss.read(&chunkHeader[0], 4);
			riffSizeAccum += 4;

			uint32_t chunkSize;
			iss.read(reinterpret_cast<char *>(&chunkSize), sizeof(chunkSize));
			chunkSize = swaple(chunkSize);
			riffSizeAccum += sizeof(chunkSize);

			if(riffSize - riffSizeAccum < chunkSize) {
				debugmanager()->fatal(
				    "data chunk size does not fit into riff size");
			}

			if(chunkHeader == "fmt ") {
				if(chunkSize != 16) {
					debugmanager()->fatal("format chunk size must be 16 (PCM)");
				}

				uint16_t formatTag;
				iss.read(reinterpret_cast<char *>(&formatTag),
				         sizeof(formatTag));
				formatTag = swaple(formatTag);
				if(formatTag != 1) {
					debugmanager()->fatal("format tag must be PCM");
				}
				riffSizeAccum += sizeof(formatTag);

				uint16_t numChannels;
				iss.read(reinterpret_cast<char *>(&numChannels),
				         sizeof(numChannels));
				numChannels = swaple(numChannels);
				if(numChannels != 1 && numChannels != 2) {
					debugmanager()->fatal("number of channels must be 1 or 2");
				}
				riffSizeAccum += sizeof(numChannels);
				asset.stereo = numChannels == 2;

				uint32_t sampleRate;
				iss.read(reinterpret_cast<char *>(&sampleRate),
				         sizeof(sampleRate));
				sampleRate = swaple(sampleRate);
				riffSizeAccum += sizeof(sampleRate);
				asset.sampleRate = sampleRate;

				uint32_t avgByteRate;
				iss.read(reinterpret_cast<char *>(&avgByteRate),
				         sizeof(avgByteRate));
				avgByteRate = swaple(avgByteRate);
				riffSizeAccum += sizeof(avgByteRate);

				uint16_t blockAlign;
				iss.read(reinterpret_cast<char *>(&blockAlign),
				         sizeof(blockAlign));
				blockAlign = swaple(blockAlign);
				riffSizeAccum += sizeof(blockAlign);

				iss.read(reinterpret_cast<char *>(&bitsPerSample),
				         sizeof(bitsPerSample));
				bitsPerSample = swaple(bitsPerSample);
				if(bitsPerSample != 16 && bitsPerSample != 24) {
					debugmanager()->fatal("bits per sample must be 16 or 24");
				}
				riffSizeAccum += sizeof(bitsPerSample);

				uint8_t bytesPerSample = bitsPerSample >> 3;

				if(blockAlign != numChannels * bytesPerSample) {
					debugmanager()->fatal("block align is incorrect");
				}
				if(avgByteRate != sampleRate * blockAlign) {
					debugmanager()->fatal("average byte rate is incorrect");
				}
			} else if(chunkHeader == "data") {
				std::string data(chunkSize, ' ');
				iss.read(&data[0], chunkSize);
				riffSizeAccum += chunkSize;

				int bytesPerSample = bitsPerSample >> 3;

				asset.samples.resize(chunkSize / bytesPerSample);
				for(uint32_t i = 0; i < chunkSize; i += bytesPerSample) {
					int16_t sample = *(int16_t *)&data[i + 1];
					sample = swaple(sample);
					asset.samples[i / bytesPerSample] = sample;
				}

				if(chunkSize % 2 == 1) {
					if(riffSize - riffSizeAccum < 1) {
						debugmanager()->fatal(
						    "data padding byte does not fit into riff size");
					}
					iss.ignore(1);
					++riffSizeAccum;
				}
			} else if(chunkHeader == "cue ") {
				uint32_t numCues;
				iss.read(reinterpret_cast<char *>(&numCues), sizeof(numCues));
				numCues = swaple(numCues);

				for(uint32_t i = 0; i < numCues; ++i) {
					uint32_t id;
					iss.read(reinterpret_cast<char *>(&id), sizeof(id));
					id = swaple(id);

					uint32_t pos;
					iss.read(reinterpret_cast<char *>(&pos), sizeof(pos));
					pos = swaple(pos);

					asset.loopPoint = pos;
					debugmanager()->info("pos = ", pos);

					iss.ignore(4);
					iss.ignore(4);
					iss.ignore(4);
					iss.ignore(4);
				}

				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "fact") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "smpl") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "acid") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "tlst") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "id3 ") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "INFO") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else if(chunkHeader == "LIST") {
				iss.ignore(chunkSize);
				riffSizeAccum += chunkSize;
			} else {
				std::stringstream ss;
				ss << "unrecognized chunk header `" << chunkHeader << "` at 0x"
				   << std::hex << riffSizeAccum;
				debugmanager()->fatal(ss.str());
			}
		}

		s << asset;
		return asset::name<asset::audio>();
	};
	converters["obj"] = [](const std::string &data, core::serializer &s) {
		asset::model asset;
		std::vector<Point3> positions;
		std::vector<Point3> normals;
		std::vector<Point2> texcoords;

		std::istringstream iss(data);
		std::string line;
		for(int iLine = 1; getline(iss, line); ++iLine) {
			std::istringstream ls(line);

			std::string directive;
			std::getline(ls, directive, ' ');
			if(directive == "v") {
				Point3 p;
				ls >> p.x >> p.y >> p.z;
				positions.emplace_back(p);
			} else if(directive == "vn") {
				Point3 n;
				ls >> n.x >> n.y >> n.z;
				normals.emplace_back(n);
			} else if(directive == "vt") {
				Point2 t;
				ls >> t.x >> t.y;
				texcoords.emplace_back(t);
			} else if(directive == "f") {
				std::string pstr, qstr, rstr;
				ls >> pstr >> qstr >> rstr;

				std::istringstream ps(pstr);
				std::istringstream qs(qstr);
				std::istringstream rs(rstr);

				asset::triangle triangle;

				int p_p, p_t, p_n;
				int q_p, q_t, q_n;
				int r_p, r_t, r_n;

				ps >> p_p;
				triangle.p.position = positions[p_p - 1];
				ps.get(); // remove slash
				if(ps.peek() != '/') {
					ps >> p_t;
					triangle.p.texcoord = texcoords[p_t - 1];
				}
				ps.get(); // remove slash
				ps >> p_n;
				triangle.p.normal = normals[p_n - 1];

				qs >> q_p;
				triangle.q.position = positions[q_p - 1];
				qs.get(); // remove slash
				if(qs.peek() != '/') {
					qs >> q_t;
					triangle.q.texcoord = texcoords[q_t - 1];
				}
				qs.get(); // remove slash
				qs >> q_n;
				triangle.q.normal = normals[q_n - 1];

				rs >> r_p;
				triangle.r.position = positions[r_p - 1];
				rs.get(); // remove slash
				if(rs.peek() != '/') {
					rs >> r_t;
					triangle.r.texcoord = texcoords[r_t - 1];
				}
				rs.get(); // remove slash
				rs >> r_n;
				triangle.r.normal = normals[r_n - 1];

				asset.triangles.emplace_back(triangle);

				while(ls.good()) {
					std::string sstr;
					ls >> sstr;

					std::istringstream ss(sstr);

					triangle.q = triangle.r;

					int s_p, s_t, s_n;

					ss >> s_p;
					triangle.r.position = positions[s_p - 1];
					ss.get(); // remove slash
					if(ss.peek() != '/') {
						ss >> s_t;
						triangle.r.texcoord = texcoords[s_t - 1];
					}
					ss.get(); // remove slash
					ss >> s_n;
					triangle.r.normal = normals[s_n - 1];

					asset.triangles.emplace_back(triangle);
				}
			} else if(directive == "f") {
				std::string pstr, qstr, rstr;
				ls >> pstr >> qstr >> rstr;

				std::istringstream ps(pstr);
				std::istringstream qs(qstr);
				std::istringstream rs(rstr);

				int p, q, r;
				ps >> p;
				qs >> q;
				rs >> r;

				asset::triangle triangle;
				triangle.p.position = positions[p - 1];
				triangle.q.position = positions[q - 1];
				triangle.r.position = positions[r - 1];

				if(normals.size() > p - 1) {
					triangle.p.normal = normals[p - 1];
				}
				if(normals.size() > q - 1) {
					triangle.q.normal = normals[q - 1];
				}
				if(normals.size() > r - 1) {
					triangle.r.normal = normals[r - 1];
				}

				if(texcoords.size() > p - 1) {
					triangle.p.texcoord = texcoords[p - 1];
				}
				if(texcoords.size() > q - 1) {
					triangle.q.texcoord = texcoords[q - 1];
				}
				if(texcoords.size() > r - 1) {
					triangle.r.texcoord = texcoords[r - 1];
				}

				asset.triangles.emplace_back(triangle);

				while(ls.good()) {
					std::string sstr;
					ls >> sstr;

					std::istringstream ss(sstr);

					int s;
					ss >> s;

					triangle.q = triangle.r;

					triangle.r.position = positions[s - 1];
					if(normals.size() > s - 1) {
						triangle.r.normal = normals[s - 1];
					}
					if(texcoords.size() > s - 1) {
						triangle.r.texcoord = texcoords[s - 1];
					}

					asset.triangles.emplace_back(triangle);
				}
			} else if(directive == "mtllib") {
				std::string mat;
				ls >> mat;

				auto find = mat.rfind(".mtl");
				if(find != std::string::npos) {
					asset.material.emplace(mat.substr(0, find));
				}
			}
		}

		s << asset;
		return asset::name<asset::model>();
	};
	converters["mtl"] = [](const std::string &data, core::serializer &s) {
		asset::material asset;

		std::istringstream iss(data);
		std::string line;
		for(int iLine = 1; getline(iss, line); ++iLine) {
			std::istringstream ls(line);

			std::string directive;
			std::getline(ls, directive, ' ');
			if(directive == "Ka") {
				ls >> asset.ambient.r >> asset.ambient.g >> asset.ambient.b;
			} else if(directive == "Kd") {
				ls >> asset.diffuse.r >> asset.diffuse.g >> asset.diffuse.b;
			} else if(directive == "Ks") {
				ls >> asset.specular.r >> asset.specular.g >> asset.specular.b;
			} else if(directive == "Ns") {
				ls >> asset.specular_exponent;
			} else if(directive == "map_Kd") {
				std::string map;
				ls >> map;

				auto find = map.rfind(".png");
				if(find != std::string::npos) {
					asset.diffuse_map.emplace(map.substr(0, find));
				}
			} else if(directive == "map_Ks") {
				std::string map;
				ls >> map;

				auto find = map.rfind(".png");
				if(find != std::string::npos) {
					asset.specular_map.emplace(map.substr(0, find));
				}
			}
		}

		s << asset;
		return asset::name<asset::material>();
	};
	converters["gls"] = [](const std::string &data, core::serializer &s) {
		asset::shaderprogram asset;
		std::ostringstream header;

		std::vector<std::tuple<std::string, std::string>> uniforms;
		std::vector<std::tuple<std::string, std::string>> attribs;
		std::vector<std::tuple<std::string, std::string, std::string>> varyings;
		std::vector<std::tuple<std::string, std::string>> outs;

		std::istringstream iss(data);
		std::string line;
		for(int iLine = 1; getline(iss, line); ++iLine) {
			auto len = line.length();
			if(len > 0) {
				if(line[0] == '@') {
					std::istringstream ls(line.substr(1));

					std::string directive;
					std::getline(ls, directive, ' ');
					if(!ls.good() && directive.empty()) {
						debugmanager()->fatal(iLine, ": missing directive");
					}

					std::vector<std::string> args;
					std::string tmp;
					while(ls.good()) {
						std::getline(ls, tmp, ' ');
						if(!tmp.empty()) { args.emplace_back(tmp); }
					}

					if(directive == "shader") { /* shader stage */
						if(args.empty()) {
							debugmanager()->fatal(iLine,
							                      ": missing shader type");
						}

						if(args[0] == "vertex") {
							asset.shaders.emplace_back(
							    support::shader::shadertype::vertex,
							    header.str());
						} else if(args[0] == "fragment") {
							asset.shaders.emplace_back(
							    support::shader::shadertype::fragment,
							    header.str());
						} else {
							debugmanager()->error(
							    iLine, ": unknown shader type `", args[0], '`');
						}
					} else if(directive == "uniform") { /* uniform variable */
						if(args.empty()) {
							debugmanager()->fatal(iLine,
							                      ": missing uniform type");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(iLine,
							                      ": missing uniform name");
						}
						asset.uniforms.emplace_back(args[1]);
						uniforms.emplace_back(args[0], args[1]);
					} else if(directive == "attrib") { /* vertex attribute*/
						if(args.empty()) {
							debugmanager()->fatal(iLine,
							                      ": missing attribute type");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(iLine,
							                      ": missing attribute name");
						}
						attribs.emplace_back(args[0], args[1]);
					} else if(directive ==
					          "varying") { /* vertex->fragment interpolable */
						if(args.empty()) {
							debugmanager()->fatal(
							    iLine,
							    ": missing varying interpolation method");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(iLine,
							                      ": missing varying type");
						}
						if(args.size() < 3) {
							debugmanager()->fatal(iLine,
							                      ": missing varying name");
						}
						varyings.emplace_back(args[0], args[1], args[2]);
					} else if(directive ==
					          "in") { /* input from previous pipeline stage */
						if(args.empty()) {
							debugmanager()->fatal(
							    iLine, ": missing program input key");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(
							    iLine, ": missing program input name");
						}
						asset.ins.emplace_back(args[0], args[1]);
						uniforms.emplace_back("sampler2D", args[1]);
					} else if(directive == "gin") { /* global input */
						if(args.empty()) {
							debugmanager()->fatal(
							    iLine, ": missing program input key");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(
							    iLine, ": missing program input name");
						}
						asset.globalIns.emplace_back(args[0], args[1]);
						uniforms.emplace_back("sampler2D", args[1]);
					} else if(directive ==
					          "out") { /* output to next pipeline stage */
						if(args.empty()) {
							debugmanager()->fatal(
							    iLine, ": missing program output type");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(
							    iLine, ": missing program output key");
						}
						if(args[0] == "rgba8") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine, ": missing program output name");
							}
							asset.outs.emplace_back(
							    support::shader::outputtype::rgba8, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb16f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine, ": missing program output name");
							}
							asset.outs.emplace_back(
							    support::shader::outputtype::rgb16f, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba16f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine, ": missing program output name");
							}
							asset.outs.emplace_back(
							    support::shader::outputtype::rgba16f, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb32f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine, ": missing program output name");
							}
							asset.outs.emplace_back(
							    support::shader::outputtype::rgb32f, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba32f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine, ": missing program output name");
							}
							asset.outs.emplace_back(
							    support::shader::outputtype::rgba32f, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "depth") {
							asset.outs.emplace_back(
							    support::shader::outputtype::depth, args[1]);
						} else {
							debugmanager()->error(
							    iLine, ": unknown program output type `",
							    args[0], '`');
						}
					} else if(directive == "gout") { /* global output */
						if(args.empty()) {
							debugmanager()->fatal(
							    iLine, ": missing program global output type");
						}
						if(args.size() < 2) {
							debugmanager()->fatal(
							    iLine, ": missing program global output key");
						}
						if(args[0] == "rgba8") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine,
								    ": missing program global output name");
							}
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::rgba8, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb16f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine,
								    ": missing program global output name");
							}
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::rgb16f, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba16f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine,
								    ": missing program global output name");
							}
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::rgba16f, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "rgb32f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine,
								    ": missing program global output name");
							}
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::rgb32f, args[1]);
							outs.emplace_back("vec3", args[2]);
						} else if(args[0] == "rgba32f") {
							if(args.size() < 3) {
								debugmanager()->fatal(
								    iLine,
								    ": missing program global output name");
							}
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::rgba32f, args[1]);
							outs.emplace_back("vec4", args[2]);
						} else if(args[0] == "depth") {
							asset.globalOuts.emplace_back(
							    support::shader::outputtype::depth, args[1]);
						} else {
							debugmanager()->error(
							    iLine, ": unknown program global output type `",
							    args[0], '`');
						}
					} else {
						debugmanager()->error(iLine, ": unknown directive `",
						                      directive, '`');
					}
				} else {
					if(asset.shaders.empty()) {
						header << line << '\n';
					} else {
						asset.shaders.back().source += line + '\n';
					}
				}
			}
		}

		for(auto &shader : asset.shaders) {
			std::string prepend;
			prepend += "#version 150\n";
			prepend += "#extension GL_ARB_explicit_attrib_location: enable\n";
			prepend += "#define Decimal float\n";
			prepend += "#define Point2 vec2\n";
			prepend += "#define Point3 vec3\n";
			prepend += "#define Point4 vec4\n";
			prepend += "#define Mat4 mat4\n";
			prepend += "precision highp float;\n";
			for(auto &uniform : uniforms) {
				prepend += "uniform " + std::get<0>(uniform) + ' ' +
				           std::get<1>(uniform) + ";\n";
			}
			if(shader.type == support::shader::shadertype::vertex) {
				int attribLoc = 0;
				for(auto &attrib : attribs) {
					prepend += "layout(location=" +
					           std::to_string(attribLoc++) + ") in " +
					           std::get<0>(attrib) + ' ' + std::get<1>(attrib) +
					           ";\n";
				}
				for(auto &varying : varyings) {
					prepend += std::get<0>(varying) + " out " +
					           std::get<1>(varying) + ' ' +
					           std::get<2>(varying) + ";\n";
				}
			} else if(shader.type == support::shader::shadertype::fragment) {
				for(auto &varying : varyings) {
					prepend += std::get<0>(varying) + " in " +
					           std::get<1>(varying) + ' ' +
					           std::get<2>(varying) + ";\n";
				}
				int outLoc = 0;
				for(auto &out : outs) {
					prepend += "layout(location=" + std::to_string(outLoc++) +
					           ") out " + std::get<0>(out) + ' ' +
					           std::get<1>(out) + ";\n";
				}
			}
			shader.source = prepend + shader.source;
		}

		s << asset;
		return asset::name<asset::shaderprogram>();
	};
	converters["lvl"] = [](const std::string &data, core::serializer &s) {
		asset::level level;
		std::vector<support::level::keyframe> kfs;

		std::istringstream iss(data);
		std::string word;

		while(iss >> word) {
			if(word[0] == '@') {
				std::istringstream issSeconds(word.substr(1));
				float seconds;
				issSeconds >> seconds;
				auto ticks = uint64_t(seconds * 10000.0);

				if(kfs.empty()) {
					kfs.emplace_back(ticks);
				} else {
					kfs.emplace_back(ticks, kfs.back());
				}
			} else if(word == "baseThreshold") {
				iss >> kfs.back().baseThreshold;
			} else if(word == "beatTicks") {
				iss >> kfs.back().beatTicks;
			} else if(word == "beatPower") {
				iss >> kfs.back().beatPower;
			} else if(word == "beatStrength") {
				iss >> kfs.back().beatStrength;
			} else if(word == "waveTicks") {
				iss >> kfs.back().waveTicks;
			} else if(word == "wavePower") {
				iss >> kfs.back().wavePower;
			} else if(word == "waveStrength") {
				iss >> kfs.back().waveStrength;
			} else if(word == "worldScale") {
				auto &worldScale = kfs.back().worldScale;
				iss >> worldScale.x >> worldScale.y >> worldScale.z;
			} else if(word == "colorTicks") {
				iss >> kfs.back().colorTicks;
			} else if(word == "colors") {
				auto &colors = kfs.back().colors;
				iss >> colors[0].r >> colors[0].g >> colors[0].b >>
				    colors[1].r >> colors[1].g >> colors[1].b >> colors[2].r >>
				    colors[2].g >> colors[2].b;
			}
		}
		for(auto &kf : kfs) { level.keyframes.emplace(kf); }
		s << level;
		return asset::name<asset::level>();
	};

	for(auto &file : files) {
		debugmanager()->info("processing `", file, '`');
		auto pos = file.find_last_of('.');
		if(pos != file.npos && pos < file.length() - 1) {
			std::string ext = file.substr(pos + 1);
			auto converter  = converters.find(ext);
			if(converter != converters.end()) {
				debugmanager()->info("found converter for `", ext, '`');

				std::string data = fs::local::read(path + "/" + file);
				std::stringstream ss;
				core::serializer serializer(ss);
				std::string type = converter->second(data, serializer);

				std::string name = file.substr(0, pos);
				fs::local::createdir(buildPath + '/' + type);
				fs::local::write(buildPath + "/" + type + "/" + name + ".asset",
				                 ss);
			} else {
				debugmanager()->warning(file,
				                        ": no appropriate converter found");
			}
		}
	}
	return 0;
}
