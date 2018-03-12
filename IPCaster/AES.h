#pragma once

#ifndef _AES_H_
#define _AES_H_

#include <Windows.h>

class CAES
{
public:
	/*
	* 功  能：初始化
	* 参  数：key — 密钥，必须是16字节(128bit)
	*/
	CAES(const BYTE key[16]);
	~CAES();

	/*
	* 功  能：加密，加密后的字节串长度只会是16字节的整数倍
	* 参  数：src_data — 需要加密的字节串，不允许为空
	*        src_len — src_data长度，不允许为0
	*        dst_data — 指向加密后字节串的指针，如果该指针为空或者dst_len小于加密后所需的字节长度，函数内部会自动分配空间
	*        dst_len — dst_data长度
	*        release_dst — 函数内部自动分配空间时是否删除现有空间
	* 返回值: 加密字节串长度
	*/
	size_t Encrypt(IN const void* const src_data, IN size_t src_len, OUT void*& dst_data, IN size_t dst_len, IN bool release_dst = false);

	/*
	* 功  能：解密
	* 参  数：data — [IN] 需要解密的字节串，不允许为空
	*                [OUT]解密后的字节串
	*        len — 字节串长度，该长度必须是16字节(128bit)的整数倍
	*/
	void Decrypt(IN OUT void* data, IN size_t len);

	/*
	* 功  能: 获取待加密的字节串被加密后字节长度
	* 参  数: src_len — 需要加密的字节串长度
	* 返回值: 加密后字节串长度
	*/
	size_t GetEncryptDataLen(IN size_t src_len) const;

private:
	// 对data前16字节进行加密
	void Encrypt(BYTE* data);
	// 对data前16字节进行解密
	void Decrypt(BYTE* data);
	// 密钥扩展
	void KeyExpansion(const BYTE* key);
	BYTE FFmul(BYTE a, BYTE b);
	// 轮密钥加变换
	void AddRoundKey(BYTE data[][4], BYTE key[][4]);
	// 加密字节替代
	void EncryptSubBytes(BYTE data[][4]);
	// 解密字节替代
	void DecryptSubBytes(BYTE data[][4]);
	// 加密行移位变换
	void EncryptShiftRows(BYTE data[][4]);
	// 解密行移位变换
	void DecryptShiftRows(BYTE data[][4]);
	// 加密列混淆变换
	void EncryptMixColumns(BYTE data[][4]);
	// 解密列混淆变换
	void DecryptMixColumns(BYTE data[][4]);

private:
	BYTE* encrypt_permutation_table_;   // 加密置换表
	BYTE* decrypt_permutation_table_;   // 解密置换表
	BYTE round_key_[11][4][4];          // 轮密钥
};

#endif // !_AES_H_


