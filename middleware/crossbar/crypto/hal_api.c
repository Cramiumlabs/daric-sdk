/**
 ******************************************************************************
 * @file    hal_api.c
 * @author  SCE Team
 * @brief   HAL API implementation file.
 *          This file provides firmware functions for big number arithmetic
 *          and primality testing.
 *
 ******************************************************************************
 * @attention
 *
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */

#include "hal_api.h"
// #ifdef DRV_SCE_ENABLED
#include <string.h>
#include <stdbool.h>
#include "ll_api.h"
#include "pke_rsa.h"
#include "bn_util.h"
// #include "daric_printf.h"

// #define HAL_API_DEBUG_EN

#if defined(HAL_API_DEBUG_EN)
#define bndebug(format, ...)     Debug_Print(format, ##__VA_ARGS__)
#define halPrintBuf(format, ...) bnu_print_mem_duart_be(format, ##__VA_ARGS__)
#else
#define bndebug(format, ...)
#define halPrintBuf(format, ...)
#endif

void big_prime_dump(const bigprime_t *bn)
{
    bnu_print_le(bn->data, safePrimeWordLen);
}

// get a rand (len words), rand < limit;
static void big_prime_random_le(uint32_t *rand, uint32_t len, const bigprime_t *lmt)
{
    if (lmt == NULL)
    {
        rng_buffer(rand, len);
        return;
    }
    // get bit length and word length of limit
    uint32_t lmtbitlen = bnu_get_msb_le(lmt->data, safePrimeWordLen) + 1;
    if (len < lmtbitlen / 32)
    {
        rng_buffer(rand, len);
        return;
    }
    memset(rand, 0, len * 4);
    rng_buffer(rand, (lmtbitlen + 31) / 32);
    rand[(lmtbitlen + 31) / 32 - 1] &= (1 << (lmtbitlen % 32)) - 1;
    // make sure rand < lmt
    if (bnu_cmp_le(rand, len, lmt->data, len) > 0)
    {
        rand[(lmtbitlen + 31) / 32 - 1] &= (1 << (lmtbitlen % 32 - 1)) - 1;
    }
}

// big number (little endian) add uint32_t (0~4G); xlen is words length, must >= 2;
// output can be same with input
void bn_add_uint32_le(const uint32_t *input, uint32_t xlen, uint32_t data, uint32_t *output)
{
    uint32_t ybuf[2] = { 0 };
    ybuf[0]          = data;
    modulo_add_le(input, ybuf, ALLF_DATA, xlen, 2, xlen, output);
}

// big number (little endian) sub uint32_t (0~4G); xlen is words length, must >= 2;
// output can be same with input
void bn_sub_uint32_le(const uint32_t *input, uint32_t xlen, uint32_t data, uint32_t *output)
{
    uint32_t ybuf[2] = { 0 };
    ybuf[0]          = data;
    modulo_sub_le(input, ybuf, ALLF_DATA, xlen, 2, xlen, output);
}

// report whether n passes reps rounds of Miller-Rabin primaltiy test
// n (little endian) is known to be non-zero
// reps is bigger than 1
int Miller_Rabin(const bigprime_t *n, int32_t reps)
{
    bndebug("%s(%d rounds) enter!\r\nn = ", __func__, reps);
    big_prime_dump(n);

    bigprime_t bntmp, nm1, nrk, rand, y, bn_one;
    memset(bntmp.data, 0, sizeof(bntmp.data));
    bntmp.data[0] = 2; // bntmp = 2;
    // if n==2, return true;
    if (bnu_cmp_le(n->data, safePrimeWordLen, bntmp.data, safePrimeWordLen) == 0)
    {
        return true;
    }
    // if n is even, return false;
    if ((n->data[0] & 1) == 0)
    {
        return false;
    }
    // nm1 = n - 1
    bn_sub_uint32_le(n->data, safePrimeWordLen, 1, nm1.data);
    // bndebug("nm1 = ");big_prime_dump(&nm1);
    // bntmp = nm1 >> k, bntmp is odd
    bnu_copy(nm1.data, safePrimeWordLen, nrk.data);
    uint32_t k = 0;
    while ((nrk.data[0] & 1) == 0)
    {
        bnu_rightshift_le(nrk.data, safePrimeWordLen, 1, bntmp.data);
        bnu_copy(bntmp.data, safePrimeWordLen, nrk.data);
        k++;
        // bndebug("nm1 >> %d = ", k);big_prime_dump(&nrk);
    }
    bndebug("nm1 = ");
    big_prime_dump(&nm1);
    bndebug("mrk is nm1>>(k=%d) = ", k);
    big_prime_dump(&nrk);

    // big number one in little endian
    memset(bn_one.data, 0, sizeof(bn_one.data));
    bn_one.data[0] = 1;
    // Test "reps" rounds
    while (reps-- > 0)
    {
        uint32_t ps = 0;
        // Gen a random < n
        big_prime_random_le(rand.data, safePrimeWordLen, n);
        bndebug("random(%d) = ", reps);
        big_prime_dump(&rand);
        // y = rand^nrk mod n
        modulo_expo_le(rand.data, nrk.data, n->data, safePrimeWordLen, safePrimeWordLen, safePrimeWordLen, y.data);
        bndebug("y (rand^nrk mod n) = ");
        big_prime_dump(&y);
        if ((bnu_cmp_le(y.data, safePrimeWordLen, bn_one.data, safePrimeWordLen) == 0) || (bnu_cmp_le(y.data, safePrimeWordLen, nm1.data, safePrimeWordLen) == 0))
        {
            continue; // if y = 1 or y = n-1, pass this random round
        }
        // y is NOT 1 or n-1, y = y*y loop k-1 times
        for (uint32_t j = 1; j < k; j++)
        {
            modulo_multiply_le(y.data, y.data, n->data, safePrimeWordLen, safePrimeWordLen, safePrimeWordLen, y.data);
            bndebug("y^(%d*2) = ", j);
            big_prime_dump(&y);
            // if y = n-1, pass this random round
            if (bnu_cmp_le(y.data, safePrimeWordLen, nm1.data, safePrimeWordLen) == 0)
            {
                ps = 1;
                break;
            }
            // if y = 1, return false, n is NOT prime;
            if (bnu_cmp_le(y.data, safePrimeWordLen, bn_one.data, safePrimeWordLen) == 0)
            {
                return false;
            }
        }
        if (ps != 1)
        {
            return false;
        }
        ps = 0;
    }
    bndebug("%s() PASS!\r\n", __func__);
    return true;
}

// smallPrimesProduct(65bit) is 16294579238595022365 = 0xE221F97C30E94E1D
static uint32_t smallPrimesProduct_64_le[] = { 0x30E94E1D, 0xE221F97C };
static uint8_t  smallPrimes[]              = { 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53 };

// P = 2*Q + 1; if Q is prime and 'a^(P-1) mod P = 1', a coprime with P, then P is prime
int isPocklingtonCriterionSatisfied(const bigprime_t *P)
{
    uint32_t   tmpbuf[safePrimeWordLen + 2] = { 0 };
    bigprime_t bn_one;

    // big number one in little endian
    memset(bn_one.data, 0, sizeof(bn_one.data));
    bn_one.data[0] = 1;

    gcd_le(P->data, smallPrimesProduct_64_le, safePrimeWordLen, 2, tmpbuf);
    // bndebug("sprime = %d:\t", smallPrimesProduct_64_le[0]); bnu_print_mem_duart_be("gcd", (uint8_t*)tmpbuf, 8);
    if ((tmpbuf[0] == 1) && (tmpbuf[1] == 0))
    {
        // a coprime with P, check 'a^(P-1) mod P = 1'
        bn_sub_uint32_le(P->data, safePrimeWordLen, 1, tmpbuf);
        bndebug("P-1 = ");
        big_prime_dump((bigprime_t *)tmpbuf);
        modulo_expo_le(smallPrimesProduct_64_le, tmpbuf, P->data, 3, safePrimeWordLen, safePrimeWordLen, tmpbuf);
        bndebug("sprime^(P-1) mod P = ");
        big_prime_dump((bigprime_t *)tmpbuf);
        if (bnu_cmp_le(tmpbuf, safePrimeWordLen, bn_one.data, safePrimeWordLen) == 0)
        {
            return true;
        }
    }
    return false;
}

// preliminary primality test: check Q whether coprime to small primes;
static int isPrimeCandidate(const bigprime_t *Q)
{
    uint32_t i;
    uint32_t sprime[2]                  = { 0 };
    uint32_t tmpR[safePrimeWordLen + 2] = { 0 };

    for (i = 0; i < sizeof(smallPrimes) / sizeof(smallPrimes[0]); i++)
    {
        sprime[0] = smallPrimes[i];
        bn_gcd(Q->data, sprime, safePrimeWordLen, 2, tmpR); // bntmp = gcd(Q, small prime)
        // bndebug("sprime = %d:\t", sprime[0]); bnu_print_mem_duart_be("gcd = ", (uint8_t*)tmpR, 8);
        if ((tmpR[0] == 1) && (tmpR[1] == 0))
        {
            continue; // gcd is 1, so Q coprime to this small prime, try next
        }
        return false; // gcd is Not 1, so Q is NOT coprime to this small prime, Q is not prime
    }
    return true;
}

#define SAFEPRIME_TEST

#if defined(TEST_SCE_ENABLED) && defined(SAFEPRIME_TEST)
static char *primeQ_1024[] = {
    "61E8078FCAF77F16CE3EF7889C6B09B0BFE3942E67A0787627A7330DE0EA82711B39FAC3AA8DC4EB4419CB7A1BAFEFE4CCCFD4EB56BBA0ABD4C3695F81005F41F37C19FD58E25AC3CAA250D6B29446C3BFF9919E5B1D5A"
    "EF3E7CDE7ED0E187A7A8459C73D14A900C65C872E71EF0A040613E222E2C1A858A48F4306DB544938D",
    "7809130A4B2922623B740FD8D431B2C4C63CB48F8F8CEC13945C5A1E740E59110385CF353D10496FF61BB8C8DE2FC652E65E587318B8B6BB8C8C15CC38B6D2BBFE5E7B4095DCCA8AE29A8BE1C08FCC692F46DC01BF701B"
    "A6DE18DFAD99ED201B2B26488D69042E1520010737DF707E399B3A93AD4F85160856AED728D8E48F73",
    "7A2E6E0954BD492EF72E8E5D5BD7E9F2C9B7BFD0A47378400009145B4BD056D735B609F01FE90A662FCCC9B4BBDC25DF959E4AD8A5F347C3D546B5E6A09645EBCDC76F370DB92A7DBFF2A2827138555C7D59C4068BAB1A"
    "7203FF27D4B3CE6AC17296B8C1E605FF9A0E9C920CF5B2500DC780F738527A5C8FC3ADDFD748C52AE9",
    "79F7911803537D06BA35E8EF3154688C3136F98E75E81255E986A5FFE4F5504F8AF789F53F2BEBFA7D4DC02A2EAC33E19B51585EA77209FD39A3B524B3A3B5635E307602C27D919D3D56B968F6367A7CDC1AB91DCDBB62"
    "2584BDF0CC02D25980F07246574C0817D17F45B3AA63EE43BDB8E33DF4B2E76B43FE9AF9EFA44ED89F",
    "7310CE082E8A246308F692D06AB1515749D2C1D3CCBFDC1DEC9091ED17AFFC9CF8D75E9D21809ED53F12B3782157724506832535F1AC607E2670C14654D06ED1FE0276BD8C7D3D8164E9B6B7DD8D15A170EEC1671A9AF2"
    "966AB4F4B8D8BF28F0559E27E08FB6E39111C3218E0674C664A7984D41C836643E01AAE8262AD7C6A5",
    "67627615EB5115E0D01441B0E8DFE6DCC5C7D0EB00DA75B4314E08B96969B591713D74564B8A10EA44E35BAEA9388468745954F9533FA0B9F76D34E2CA6109F9B250316B6D01691ACC3D93738F5CA8C20348E4A4B288D7"
    "4F9BC5E470A0BC1DD70A872FBEE4B54A22A154CBE49F34A15E22301C88B38783E6CDC9F103CD3213FD",
    "7927EE4DA8500156BA81EC55C935F085C4974DBDFD9C666FEC194178BE4666A59DFFC5029AB842B12F204BBEB75F8BB18133E434DAB18BB363F7240C700042EF09EB0852D1D7F378CABE370D2513A52915F44820C989F3"
    "287351393E73585C3549717C0D75A25CE3C76FC8E80CC35A0EC460BFD247817437F8B7CFB2A5A4DC89",
    "69606D416178A8F032F9E83CBE33A4C221F5124DD140E05962F389DB6600A2FFA7D0D68154BF030055D61614B0A03C87CE156FB0B7A809206D1CEE9AEBE7B7F94B0B97E47477089AE4D33EFF1F3FFE2E48CDC468C2E00C"
    "DC350D71B9490DFF159843A50403DE4C5D785340207B8E61457A33EF60FB6A2FC9C2D707D96105982B",
    "7AA50ED6DB57082FBC0DC5DFF44A2F4C9813502E518CEF238F608FFB7620736F5BF83BC8D6EBA1CC3C4C41C76EE725F25F32890E2CD0F1B14E270D1FD21DE00132EB54367A1281D501D7B7D54D6887E515482F1D0C763D"
    "D2A9CD1F5C7B1A7F8C6238287B9444C69FABC1867302BEA1BAD187B815D23927D66B2FDF164EA5C7A7",
};

static char *primeP_1024[] = {
    "C3D00F1F95EEFE2D9C7DEF1138D613617FC7285CCF40F0EC4F4E661BC1D504E23673F587551B89D6883396F4375FDFC9999FA9D6AD774157A986D2BF0200BE83E6F833FAB1C4B5879544A1AD65288D877FF3233CB63AB5"
    "DE7CF9BCFDA1C30F4F508B38E7A2952018CB90E5CE3DE14080C27C445C58350B1491E860DB6A89271F",
    "F0122614965244C476E81FB1A86365898C79691F1F19D82728B8B43CE81CB222070B9E6A7A2092DFEC377191BC5F8CA5CCBCB0E631716D7719182B98716DA577FCBCF6812BB99515C53517C3811F98D25E8DB8037EE037"
    "4DBC31BF5B33DA4036564C911AD2085C2A40020E6FBEE0FC733675275A9F0A2C10AD5DAE51B1C91EEB",
    "F45CDC12A97A925DEE5D1CBAB7AFD3E5936F7FA148E6F080001228B697A0ADAE6B6C13E03FD214CC5F99936977B84BBF2B3C95B14BE68F87AA8D6BCD412C8BD79B8EDE6E1B7254FB7FE54504E270AAB8FAB3880D175634"
    "E407FE4FA9679CD582E52D7183CC0BFF341D392419EB64A01B8F01EE70A4F4B91F875BBFAE918A55D7",
    "F3EF223006A6FA0D746BD1DE62A8D118626DF31CEBD024ABD30D4BFFC9EAA09F15EF13EA7E57D7F4FA9B80545D5867C336A2B0BD4EE413FA73476A4967476AC6BC60EC0584FB233A7AAD72D1EC6CF4F9B835723B9B76C4"
    "4B097BE19805A4B301E0E48CAE98102FA2FE8B6754C7DC877B71C67BE965CED687FD35F3DF489DB143",
    "E6219C105D1448C611ED25A0D562A2AE93A583A7997FB83BD92123DA2F5FF939F1AEBD3A43013DAA7E2566F042AEE48A0D064A6BE358C0FC4CE1828CA9A0DDA3FC04ED7B18FA7B02C9D36D6FBB1A2B42E1DD82CE3535E5"
    "2CD569E971B17E51E0AB3C4FC11F6DC7222386431C0CE98CC94F309A83906CC87C0355D04C55AF8D4F",
    "CEC4EC2BD6A22BC1A0288361D1BFCDB98B8FA1D601B4EB68629C1172D2D36B22E27AE8AC971421D489C6B75D527108D0E8B2A9F2A67F4173EEDA69C594C213F364A062D6DA02D235987B26E71EB951840691C9496511AE"
    "9F378BC8E141783BAE150E5F7DC96A944542A997C93E6942BC44603911670F07CD9B93E2079A6427FF",
    "F24FDC9B50A002AD7503D8AB926BE10B892E9B7BFB38CCDFD83282F17C8CCD4B3BFF8A05357085625E40977D6EBF17630267C869B5631766C7EE4818E00085DE13D610A5A3AFE6F1957C6E1A4A274A522BE890419313E6"
    "50E6A2727CE6B0B86A92E2F81AEB44B9C78EDF91D01986B41D88C17FA48F02E86FF16F9F654B49B917",
    "D2C0DA82C2F151E065F3D0797C67498443EA249BA281C0B2C5E713B6CC0145FF4FA1AD02A97E0600ABAC2C296140790F9C2ADF616F501240DA39DD35D7CF6FF296172FC8E8EE1135C9A67DFE3E7FFC5C919B88D185C019"
    "B86A1AE372921BFE2B30874A0807BC98BAF0A68040F71CC28AF467DEC1F6D45F9385AE0FB2C20B305B",
    "F54A1DADB6AE105F781B8BBFE8945E993026A05CA319DE471EC11FF6EC40E6DEB7F07791ADD743987898838EDDCE4BE4BE65121C59A1E3629C4E1A3FA43BC00265D6A86CF42503AA03AF6FAA9AD10FCA2A905E3A18EC7B"
    "A5539A3EB8F634FF18C47050F728898D3F57830CE6057D4375A30F702BA4724FACD65FBE2C9D4B8F53",
};
#endif

// P = 2*Q + 1; Q is prime, P is Safe Prime; bit length: safePrimeBitLen; reps is rounds of Miller_Rabin, must > 1
int GetSafePrime(bigprime_t *P, bigprime_t *Q, int32_t reps)
{
    uint32_t        i, delta;
    uint32_t        sprime[2]                  = { 0 };
    uint32_t        tmpR[safePrimeWordLen + 2] = { 0 };
    bigprime_t      bntmp;
    static uint32_t pgtest = 0;

#if defined(TEST_SCE_ENABLED) && defined(SAFEPRIME_TEST)
    uint32_t testdatagroup = sizeof(primeQ_1024) / sizeof(primeQ_1024[0]);
#endif

    while (1)
    {
        // step 1, random Q, odd, bit length: safePrimeBitLen-1, msb is 3;
#ifdef SAFEPRIME_TEST
        bndebug("\r\n>>> step 1 [Test]\r\n");
#ifdef TEST_SCE_ENABLED
        bnu_hex2buf_le(primeQ_1024[pgtest % testdatagroup], (uint8_t *)Q->data, safePrimeByteLen);
#endif
#else
        bndebug("\r\n>>> step 1\r\n");
        big_prime_random_le(Q->data, safePrimeWordLen, NULL);
        Q->data[safePrimeWordLen - 1] &= (uint32_t)(1 << 31) - 1;
        Q->data[safePrimeWordLen - 1] |= (uint32_t)(3 << 29);
        Q->data[0] |= 1;
#endif
        bndebug("Q = ");
        big_prime_dump(Q);
        // step 2, preliminary primality test: check Q whether coprime to small primes;
        bndebug("\r\n>>> step 2\r\n");
        sce_ram_clean(); // Clear SCE RAM
        for (delta = 2; delta < (1 << 20); delta += 2)
        {
            bn_add_uint32_le(Q->data, safePrimeWordLen, 2, Q->data); // Q=Q+delta
            bndebug("==> delta = %x\r\n", delta);
            // bndebug("Q = ");big_prime_dump(Q);
            for (i = 0; i < sizeof(smallPrimes) / sizeof(smallPrimes[0]); i++)
            {
                sprime[0] = smallPrimes[i];
                bn_gcd(Q->data, sprime, safePrimeWordLen, 2, bntmp.data); // bntmp = gcd(Q, small prime)
                // bndebug("sprime = %d:\t", sprime[0]); bnu_print_mem_duart_be("gcd = ", (uint8_t*)bntmp.data, 8);
                if ((bntmp.data[0] == 1) && (bntmp.data[1] == 0))
                {
                    continue; // gcd is 1, so Q coprime to this small prime, try next;
                }
                break;
            }
            if (i == sizeof(smallPrimes) / sizeof(smallPrimes[0]))
            {
                // coprime to all small primes
                // step 3, If 'Q = 1 (mod 3)', P = 2*Q + 1 is obviously not prime
                bndebug("\r\n>>> step 3\r\n");
                bn_sub_uint32_le(Q->data, safePrimeWordLen, 1, bntmp.data); // bntmp = Q - 1
                // bndebug("Q-1 = ");big_prime_dump(&bntmp);
                sprime[0] = 3;                                               // sprime = 3
                bn_gcd(bntmp.data, sprime, safePrimeWordLen, 2, bntmp.data); // bntmp = gcd(Q-1, 3)
                // bndebug("sprime = %d:\t", sprime[0]); bnu_print_mem_duart_be("gcd = ", (uint8_t*)bntmp.data, 8);
                if ((bntmp.data[0] == 3) && (bntmp.data[1] == 0))
                {
                    continue; // gcd is 3 => '(Q-1)%3 = 0' => 'Q = 1 (mod 3)'
                }
                // Q pass the preliminary test, P = 2*Q + 1
                bnu_leftshift_le(Q->data, safePrimeWordLen, 1, P->data);
                bn_add_uint32_le(P->data, safePrimeWordLen, 1, P->data);
                bndebug("P = ");
                big_prime_dump(P);
                // step 4, preliminary primality test of P
                bndebug("\r\n>>> step 4\r\n");
                if (isPrimeCandidate(P))
                {
                    break; // P pass the preliminary test too
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        if (delta >= (1 << 20))
        {
            continue; // preliminary test fail, try new random
        }

        // step 5, Miller-Rabin
        bndebug("\r\n>>> step 5\r\n");
        if (isPocklingtonCriterionSatisfied(P) && Miller_Rabin(Q, reps))
        {
            bndebug("\r\nLucky! Get the P and Q!!\r\n");
            break; // Lucky!!
        }
        else
        {
            continue; // Q, P fail at Miller-Rabin, try new random
        }
    }
#ifdef SAFEPRIME_TEST
#ifdef TEST_SCE_ENABLED
    bnu_hex2buf_le(primeP_1024[pgtest % testdatagroup], (uint8_t *)tmpR, safePrimeByteLen);
#endif
    pgtest++;
    if (bnu_cmp_le(tmpR, safePrimeWordLen, P->data, safePrimeWordLen) == 0)
    {
        bndebug("\r\nGood! P and Q Pass the Test!!\r\n");
    }
#endif
    return 0;
}

// result = B^E mod N, B<N, E>N (extern-modular); will seperate E to calculate by intern-modular;
// length of result = n_len; b_len, e_len, n_len are words number; little endian; support to 4096b
int mod_exp_ext(uint32_t *B, uint32_t *E, uint32_t *N, uint32_t b_len, uint32_t e_len, uint32_t n_len, uint32_t *result)
{
    uint32_t E1[4096 / 32]   = { 0 };
    uint32_t E2[4096 / 32]   = { 0 };
    uint32_t tmp[4096 / 32]  = { 0 };
    uint32_t tmp1[4096 / 32] = { 0 };
    int      Eb2, thisEb2;
    uint32_t nbitlen = bnu_get_msb_le(N, n_len);
    /*
        halPrintBuf("N", (uint8_t*)N, 128);
        halPrintBuf("B", (uint8_t*)B, 128);
        halPrintBuf("E", (uint8_t*)E, 128);
    */
    if (bnu_cmp_le(E, e_len, N, n_len) < 0)
    {
        modulo_expo_le(B, E, N, b_len, e_len, n_len, result);
        return 0;
    }

    // E = E1||E2, E1 length is n_len; E2 length is (e_len - n_len); E = E1*(1<<Eb2)+E2;
    Eb2 = bnu_get_msb_le(E, e_len) - nbitlen + 1; // bit length of E2
    Eb2 = (Eb2 > 64) ? Eb2 : 65;
    bnu_rightshift_le(E, e_len, Eb2, E1); // E1, n_len
    bnu_leftshift_le(E1, n_len, Eb2, tmp);
    modulo_sub_le(E, tmp, ALLF_DATA, e_len, e_len, e_len, E2); // E2, e_len-n_len
    /*
        halPrintBuf("E1", (uint8_t*)E1, 128);
        halPrintBuf("E2", (uint8_t*)E2, 128);
        halPrintBuf("tmp", (uint8_t*)tmp, 128);
        bndebug("Eb2 = %d; E2_msb = %d\n", Eb2, bnu_get_msb_le(E2, e_len));
    */
    // B^E mod N = (B^E1)^(1<<Eb2) * B^E2 mod N = ( ((B^E1 mod N)^(1<<Eb2) mod N ) * (B^E2 mod N) ) mod N
    memset(tmp, 0, sizeof(tmp));
    mod_exp_ext(B, E2, N, b_len, (Eb2 + 31) / 32, n_len, tmp); // tmp = B^E2 mod N
    // halPrintBuf("B^E2 mod N", (uint8_t*)tmp, 64);
    // bndebug("Eb2 = %d\n", Eb2);

    modulo_expo_le(B, E1, N, b_len, n_len, n_len, tmp1); // tmp1 = B^E1 mod N
    // halPrintBuf("B^E1 mod N", (uint8_t*)tmp1, 64);

    while (1)
    {
        thisEb2 = (Eb2 > nbitlen) ? nbitlen : Eb2;
        memset(E1, 0, sizeof(E1));
        E1[0] = 1;
        memset(E2, 0, sizeof(E2));
        bnu_leftshift_le(E1, n_len, thisEb2, E2); // E2 = 1<<thisEb2
        // bndebug("thisEb2 = %d\n", thisEb2);
        // halPrintBuf("1<<thisEb2", (uint8_t*)E2, 64);

        memset(E1, 0, sizeof(E1));
        modulo_expo_le(tmp1, E2, N, n_len, n_len, n_len, E1); // E1 = (B^E1 mod N)^(1<<thisEb2) mod N
        // halPrintBuf("(B^E1 mod N)^(1<<thisEb2) mod N", (uint8_t*)E1, 64);
        Eb2 -= thisEb2;
        if (Eb2 > 0)
        {
            memcpy(tmp1, E1, n_len * 4);
        }
        else
        {
            break;
        }
    }

    modulo_multiply_le(E1, tmp, N, n_len, n_len, n_len, result);
    // halPrintBuf("( ((B^E1 mod N)^(1<<Eb2) mod N ) * (B^E2 mod N) ) mod N", (uint8_t*)result, 64);
    // bndebug("\n\n");

    return Eb2;
}

// result = X mod N; support to 4096b
// x_len, n_len are words number; little endian;
// output: words number of result;
int hal_mod(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len, uint32_t *result)
{
    uint32_t ret = 0;
    bndivision_le(X, x_len, N, n_len, NULL, NULL, result, &ret);
    return ret;
}

// result = X // Y; support to 4096b
// x_len, y_len are words number; little endian;
// output: words number of result;
int hal_floordiv(uint32_t *X, uint32_t *Y, uint32_t x_len, uint32_t y_len, uint32_t *result)
{
    uint32_t ret = 0;
    bndivision_le(X, x_len, Y, y_len, result, &ret, NULL, NULL);
    return ret;
}

// result = X+Y mod N; support to 4096b; support intern-modular and extern-modular
// x_len, y_len, n_len are words numbers; all data are little endian
int hal_add(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result)
{
    int32_t   xncmp              = bnu_cmp_le(X, x_len, N, n_len);
    int32_t   yncmp              = bnu_cmp_le(Y, y_len, N, n_len);
    uint32_t  tmpbuf1[4096 / 32] = { 0 };
    uint32_t  tmpbuf2[4096 / 32] = { 0 };
    uint32_t *pX                 = X;
    uint32_t  pX_len             = x_len;
    uint32_t *pY                 = Y;
    uint32_t  pY_len             = y_len;
    /*
        halPrintBuf("hal_add\nX", (uint8_t*)X, x_len*4);
        halPrintBuf("Y", (uint8_t*)Y, y_len*4);
        halPrintBuf("N", (uint8_t*)N, n_len*4);
    */
    if (xncmp >= 0)
    {
        pX_len = hal_mod(X, N, x_len, n_len, tmpbuf1);
        pX     = tmpbuf1;
    }
    if (yncmp >= 0)
    {
        pY_len = hal_mod(Y, N, y_len, n_len, tmpbuf2);
        pY     = tmpbuf2;
    }
    // halPrintBuf("Xn", (uint8_t*)pX, pX_len*4);
    // halPrintBuf("Yn", (uint8_t*)pY, pY_len*4);
    return modulo_add_le(pX, pY, N, pX_len, pY_len, n_len, result);
}

// result = X-Y mod N; support to 4096b; support intern-modular and extern-modular
// x_len, y_len, n_len are words numbers; all data are little endian
int hal_sub(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result)
{
    int32_t   xncmp              = bnu_cmp_le(X, x_len, N, n_len);
    int32_t   yncmp              = bnu_cmp_le(Y, y_len, N, n_len);
    uint32_t  tmpbuf1[4096 / 32] = { 0 };
    uint32_t  tmpbuf2[4096 / 32] = { 0 };
    uint32_t *pX                 = X;
    uint32_t  pX_len             = x_len;
    uint32_t *pY                 = Y;
    uint32_t  pY_len             = y_len;
    /*
        halPrintBuf("hal_sub\nX", (uint8_t*)X, x_len*4);
        halPrintBuf("Y", (uint8_t*)Y, y_len*4);
        halPrintBuf("N", (uint8_t*)N, n_len*4);
    */
    if (xncmp >= 0)
    {
        pX_len = hal_mod(X, N, x_len, n_len, tmpbuf1);
        pX     = tmpbuf1;
    }
    if (yncmp >= 0)
    {
        pY_len = hal_mod(Y, N, y_len, n_len, tmpbuf2);
        pY     = tmpbuf2;
    }
    // halPrintBuf("Xn", (uint8_t*)pX, pX_len*4);
    // halPrintBuf("Yn", (uint8_t*)pY, pY_len*4);
    return modulo_sub_le(pX, pY, N, pX_len, pY_len, n_len, result);
}

// result = X*Y mod N; support to 4096b; support intern-modular and extern-modular
// x_len, y_len, n_len are words numbers; all data are little endian
int hal_multiply(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result)
{
    int32_t   xncmp              = bnu_cmp_le(X, x_len, N, n_len);
    int32_t   yncmp              = bnu_cmp_le(Y, y_len, N, n_len);
    uint32_t  tmpbuf1[4096 / 32] = { 0 };
    uint32_t  tmpbuf2[4096 / 32] = { 0 };
    uint32_t *pX                 = X;
    uint32_t  pX_len             = x_len;
    uint32_t *pY                 = Y;
    uint32_t  pY_len             = y_len;

    if (xncmp >= 0)
    {
        pX_len = hal_mod(X, N, x_len, n_len, tmpbuf1);
        pX     = tmpbuf1;
    }
    if (yncmp >= 0)
    {
        pY_len = hal_mod(Y, N, y_len, n_len, tmpbuf2);
        pY     = tmpbuf2;
    }
    return modulo_multiply_le(pX, pY, N, pX_len, pY_len, n_len, result);
}

// result = 1/X mod N; support to 4096b; support intern-modular and extern-modular
// x_len, n_len are words numbers; all data are little endian
int hal_inverse(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len, uint32_t *result)
{
    uint32_t *pX                 = X;
    uint32_t  pX_len             = x_len;
    int32_t   xncmp              = bnu_cmp_le(X, x_len, N, n_len);
    uint32_t  tmpbuf1[4096 / 32] = { 0 };

    // halPrintBuf("hal_inverse\nX", (uint8_t*)X, x_len*4);
    // halPrintBuf("N", (uint8_t*)N, n_len*4);
    if (xncmp >= 0)
    {
        pX_len = hal_mod(X, N, x_len, n_len, tmpbuf1);
        pX     = tmpbuf1;
    }
    // halPrintBuf("pX", (uint8_t*)pX, pX_len*4);

    return modulo_inverse_le(pX, N, pX_len, n_len, result);
}

// result = B^E mod N; support intern-modular and extern-modular
// length of result = n_len; b_len, e_len, n_len are words number; little endian;
// if N is Prime, NisPrime = 1; else NisPrime = 0; used for improve performance
int hal_exp(uint32_t *B, uint32_t *E, uint32_t *N, uint32_t b_len, uint32_t e_len, uint32_t n_len, uint32_t *result, uint32_t NisPrime)
{
    int32_t   bncmp              = bnu_cmp_le(B, b_len, N, n_len);
    int32_t   encmp              = bnu_cmp_le(E, e_len, N, n_len);
    uint32_t  Bbuf[4096 / 32]    = { 0 };
    uint32_t  tmpbuf1[4096 / 32] = { 0 };
    uint32_t  tmpbuf2[4096 / 32] = { 0 };
    uint32_t *pB                 = B;
    uint32_t  pB_len             = b_len;
    uint32_t  tmplen;

    if (bncmp == 0)
    {
        // B = N, result is 0
        memset(result, 0, n_len);
        return 0;
    }
    if (bncmp > 0)
    {
        // B > N, Bbuf = B mod N
        pB_len = hal_mod(B, N, b_len, n_len, Bbuf);
        pB     = Bbuf;
    }

    if (encmp < 0)
    {
        // E < N, intern-modular
        modulo_expo_le(pB, E, N, pB_len, e_len, n_len, result);
    }
    else
    {
        // E > N, extern-modular
        if (NisPrime == 1)
        {
            // N is Prime: B^E mod N = B^(E mod N-1) mod N
            bn_sub_uint32_le(N, n_len, 1, tmpbuf1);                        // tmpbuf1 = N-1
            tmplen = hal_mod(E, tmpbuf1, e_len, n_len, tmpbuf2);           // tmpbuf2 = E mod N-1
            modulo_expo_le(pB, tmpbuf2, N, pB_len, tmplen, n_len, result); // result = pB^(E mod N-1) mod N
        }
        else
        {
            // N is not prime, seperate E (E1||E2): E = E1*(1<<b2) + E2, E1<N, E2<N, bit length of E2 is Eb2,
            // B^E mod N = (B^E1)^(1<<Eb2) * B^E2 mod N = ( ((B^E1 mod N)^(1<<Eb2) mod N ) * (B^E2 mod N) ) mod N
            mod_exp_ext(B, E, N, b_len, e_len, n_len, result);
        }
    }
    return 0;
}

// result = floor(sqrt(A)), length is awlen/2; little endian; TBD: Not implement yet!
int hal_sqrt(uint32_t *A, uint32_t awlen, uint32_t *result)
{
    return 0;
}

// Legendre Symbol for checking quadratic residue, N must be prime;
// return 1, if X is quadratic residue of N; return -1, if X is quadratic non-residue of N;
// x_len, n_len are words number; little endian
int LegendreSymbol(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len)
{
    int      ret                = -1;
    uint32_t tmpbuf1[4096 / 32] = { 0 };
    uint32_t tmpbuf2[4096 / 32] = { 0 };
    // halPrintBuf("LS_X", (uint8_t*)X, x_len*4);
    // halPrintBuf("LS_N", (uint8_t*)N, n_len*4);
    bn_sub_uint32_le(N, n_len, 1, tmpbuf1);        // tmpbuf1 = N-1
    bnu_rightshift_le(tmpbuf1, n_len, 1, tmpbuf2); // tmpbuf2 = (N-1)/2
    memset(tmpbuf1, 0, sizeof(tmpbuf1));
    // R = X^((N-1)/2) mod N
    hal_exp(X, tmpbuf2, N, x_len, n_len, n_len, tmpbuf1, 0);
    if ((tmpbuf1[0] == 1) && (tmpbuf1[1] == 0) && (tmpbuf1[3] == 0))
    {
        ret = 1;
    }
    // halPrintBuf("LS_R", (uint8_t*)tmpbuf1, n_len*4);
    return ret;
}

// #endif
