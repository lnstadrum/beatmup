/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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
STRINGIFY(
        uniform highp uint inputStride;
        layout(binding = 0, std430) readonly buffer inFeatures_ { highp uint data[][8]; } inFeatures;
        layout(binding = 0, rgba8) uniform writeonly lowp image2D[6] outFeatures;

        void main() {
            lowp vec4 f[8];
            f[0] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][0]);
            f[1] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][1]);
            f[2] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][2]);
            f[3] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][3]);
            f[4] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][4]);
            f[5] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][5]);
            f[6] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][6]);
            f[7] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][7]);
            mediump vec4 sum;
            sum = vec4(0.206178, -0.018126, 0.144261, -0.137801);
            sum += f[0] * mat4(0.340077,0.012986,0.002155,0.128053,-0.337266,-0.038149,-0.189716,-0.252363,-0.363181,-0.441258,-0.181627,-0.130080,-0.052178,-0.204372,-0.155512,1.494118);
            sum += f[1] * mat4(-0.209341,0.278133,0.050068,0.162571,0.242205,-1.110752,-0.183262,0.425787,-0.021508,-1.361506,-0.170597,0.243897,0.285674,-0.034731,-0.397750,-1.443178);
            sum += f[2] * mat4(-0.041358,-0.194016,0.371885,0.060833,-0.079068,0.068964,0.020376,0.044668,-0.160903,-0.037149,-0.004740,0.155115,0.044737,0.605865,-0.396711,-0.371307);
            sum += f[3] * mat4(-0.331231,-0.063356,0.037188,0.024671,0.222539,0.016167,-0.199047,0.069411,-0.035082,-0.012949,-0.069771,0.057520,-0.378006,-0.068015,-0.169999,0.141307);
            sum += f[4] * mat4(0.208694,0.024387,-0.080705,-0.092614,-0.090661,0.377338,-0.021183,0.122276,-0.327657,0.137176,-0.279556,-0.416470,0.011951,0.181335,-0.087424,-0.249516);
            sum += f[5] * mat4(0.008807,0.105975,0.026921,-0.108943,0.015716,0.297650,-0.363542,-0.133707,-0.279067,0.205233,-1.152121,0.114844,-0.119870,0.274859,-0.146368,-0.163447);
            sum += f[6] * mat4(0.114139,0.139128,0.105819,-0.290090,-0.344049,-0.297957,-0.067678,-0.432136,-0.001405,0.594953,-0.259244,-0.381970,-0.936454,-0.210973,-0.037248,-0.228432);
            sum += f[7] * mat4(0.121381,-0.067453,-0.045882,0.292471,-0.010840,0.194419,0.135406,0.090386,-0.188734,0.442487,-0.298218,-0.002680,-0.059592,0.624798,-0.222801,0.044306);
            imageStore(outFeatures[0], ivec2(gl_GlobalInvocationID.xy), sum);
            sum = vec4(-0.143974, 0.129822, 0.007462, -1.912489);
            sum += f[0] * mat4(-0.054715,0.139876,-0.103480,1.240505,-0.360853,-0.902970,0.195491,0.473056,-0.129030,-0.176216,0.041463,0.456359,-1.297127,-0.259475,-0.455351,0.845977);
            sum += f[1] * mat4(0.290724,-0.341261,0.304390,-1.086493,-0.018041,0.160152,-0.062883,-0.256594,0.157876,-0.272685,0.006609,-0.372259,-0.819273,0.089423,0.444207,0.941680);
            sum += f[2] * mat4(0.141597,0.004056,-0.159252,0.038627,-0.078803,-0.180448,-0.048369,-0.002029,-0.041641,-0.049141,0.159448,0.275612,-0.074305,-1.190387,0.250879,-0.209971);
            sum += f[3] * mat4(0.007640,-0.077561,-0.272518,-0.038711,0.197606,-0.068144,-0.052400,0.036364,0.074817,0.004074,-0.072470,-0.014017,-0.177867,0.701254,0.283195,-0.289139);
            sum += f[4] * mat4(-0.927356,-0.191490,0.022830,-0.163126,0.238716,0.047866,-0.394998,-0.375625,-0.012939,0.091461,-0.081925,-0.053822,-0.500817,0.127043,0.794399,0.305478);
            sum += f[5] * mat4(0.053690,-0.096710,-0.157567,0.183322,-0.647353,0.171503,-0.300473,-0.269643,-0.118353,0.059039,-0.489491,0.024664,0.057380,-1.412720,0.671397,0.901744);
            sum += f[6] * mat4(0.215520,-0.214457,-0.256192,-0.342396,-0.160441,0.360658,0.000200,-0.045428,-0.045597,0.186063,-0.333883,-0.133908,-0.267143,-0.021235,0.529059,0.068839);
            sum += f[7] * mat4(0.319489,-0.044316,-0.476471,-0.013250,-0.067110,0.153144,0.105858,-0.098055,-0.016467,-0.102739,0.162311,0.023035,-0.150250,0.102700,-0.329456,-0.214850);
            imageStore(outFeatures[1], ivec2(gl_GlobalInvocationID.xy), sum);
            sum = vec4(0.081930, 0.120358, -0.145525, 0.031622);
            sum += f[0] * mat4(-0.159689,-0.030061,-0.002726,0.395019,-0.376668,0.087704,-0.077810,0.117339,-0.129527,-0.067883,0.067023,-0.629378,-0.115404,0.032709,-0.232842,-0.425835);
            sum += f[1] * mat4(-0.014806,-0.116383,0.053914,0.592667,0.266616,-0.938465,-0.204858,0.047040,-0.091438,-0.240006,0.120099,0.576610,-0.015974,0.171075,-0.006556,0.441097);
            sum += f[2] * mat4(0.019432,0.089828,-0.053870,-0.022783,-0.105768,-0.061975,0.028279,0.136736,0.061879,0.735314,-0.381545,-0.179813,-0.205650,-0.339608,-0.279505,-0.012321);
            sum += f[3] * mat4(0.095841,0.051884,-0.013387,0.004001,0.042910,0.008607,-0.196159,0.052646,-0.328103,0.088779,-0.218873,0.028997,0.323829,-0.029416,-0.028533,-0.010326);
            sum += f[4] * mat4(-0.067041,-0.014085,0.046815,0.051227,0.014889,-0.153508,-0.018026,0.072195,0.042483,-1.307369,-0.192571,-0.219326,-0.034150,-0.599437,-0.056636,-0.118674);
            sum += f[5] * mat4(0.024772,-0.112046,0.106700,-0.007890,0.063089,0.394104,-0.442915,-0.117887,-0.026019,0.252791,-0.281324,-0.099877,0.031506,0.082491,0.051279,0.196347);
            sum += f[6] * mat4(-0.037114,-0.057818,0.037845,0.067973,0.313319,-0.005139,-0.164149,0.137804,0.196340,-0.041517,-0.062119,0.319639,1.002909,-0.311419,0.014496,-0.757798);
            sum += f[7] * mat4(-0.035899,-0.036666,0.029782,-0.075727,-0.028525,-0.935522,-0.263211,-0.028278,-0.180192,-0.621467,0.295305,-0.264024,0.131710,-1.017440,-0.045513,0.736167);
            imageStore(outFeatures[2], ivec2(gl_GlobalInvocationID.xy), sum);
            sum = vec4(-0.777585, -0.171533, -0.157531, -0.128464);
            sum += f[0] * mat4(0.877749,-0.417934,0.488598,-2.349433,0.024799,-0.134086,-0.100873,-1.028196,-0.096243,-0.042697,0.065832,0.850636,0.056862,-0.598675,-2.298410,0.010682);
            sum += f[1] * mat4(-0.557405,0.371096,0.634930,-3.018342,-0.008089,-0.005554,0.008672,0.981144,0.071592,0.077072,-0.073111,-0.730503,-0.215953,-3.460881,-0.072014,0.035777);
            sum += f[2] * mat4(-0.044135,-0.259311,-0.646963,0.075349,-0.030482,0.176840,-0.068345,-0.051291,-0.064999,0.033458,0.019260,-0.015363,-0.031595,-0.121612,-0.058116,0.008611);
            sum += f[3] * mat4(-0.316309,-0.176758,0.552900,1.123340,0.305759,0.094437,-0.016936,0.003709,0.314558,0.079398,-0.018650,0.028886,0.276172,-0.065178,-0.020839,-0.036951);
            sum += f[4] * mat4(0.477525,-0.025481,-0.611786,-0.485637,-0.101071,0.080406,-0.108456,0.164161,0.014243,0.235167,-0.056712,0.204776,-1.380003,0.085917,-0.413198,-1.092390);
            sum += f[5] * mat4(0.648327,0.432689,-0.520623,-0.722618,-0.192273,0.297280,-0.025948,0.038755,-0.123718,0.272857,0.047971,-0.142547,-0.833254,0.027404,-1.242543,0.364771);
            sum += f[6] * mat4(-0.116084,0.153554,-1.309151,-0.337194,-0.099238,0.103837,-0.143357,-0.028995,-0.008711,0.178292,-0.594058,-0.224114,-0.172938,0.312399,0.232243,-0.076795);
            sum += f[7] * mat4(-0.101037,-0.341251,-0.303487,-0.061473,-0.096822,0.054887,0.047304,-0.020771,-0.207793,-0.165513,0.045815,0.314986,-0.043108,0.031815,0.137112,-0.103813);
            imageStore(outFeatures[3], ivec2(gl_GlobalInvocationID.xy), sum);
            sum = vec4(0.292714, 0.102345, -0.612442, 0.139480);
            sum += f[0] * mat4(0.243995,0.083579,-0.021887,-0.442663,0.032167,0.072433,-0.014544,0.370772,-0.009226,0.311353,0.527441,0.658669,-0.156549,0.044782,-0.427944,-1.087501);
            sum += f[1] * mat4(0.131361,-0.059876,0.089169,-0.336182,0.002323,-0.570086,0.102247,-0.352145,0.316617,-0.913424,-0.126646,0.675033,-0.134524,-1.703220,0.246941,1.002981);
            sum += f[2] * mat4(0.002956,-0.393569,-0.093910,0.014813,-0.368153,-0.278596,0.010886,0.065438,0.091754,-0.580033,-0.031472,-0.129207,0.075036,0.354011,-0.116954,-0.118453);
            sum += f[3] * mat4(-0.293421,-0.033044,0.138627,0.207823,0.085157,-0.039530,-0.072443,0.006040,-0.146930,-0.368758,-0.119914,0.074817,-0.484242,-0.039929,-0.125076,0.016843);
            sum += f[4] * mat4(0.008556,-0.136921,0.013478,0.487618,-0.206862,-0.140686,0.020241,0.017490,0.529667,-0.292353,-0.165093,-0.227336,-0.111921,-0.255162,0.077848,-0.077834);
            sum += f[5] * mat4(0.058606,-0.075280,-0.002136,-0.094858,0.148659,0.034417,-0.359463,0.219998,0.056402,0.519925,-0.520056,-0.075257,0.101293,0.306439,-0.825092,0.047786);
            sum += f[6] * mat4(-0.039384,0.055867,0.164143,-0.066680,0.343033,-0.227716,-0.337694,-0.237103,0.034435,-0.021214,-0.320190,-0.022219,-0.025029,-0.236951,-0.155582,0.403007);
            sum += f[7] * mat4(0.059028,-0.026302,-0.067031,0.043293,0.277867,-0.188713,-0.135971,0.271548,0.144841,0.146772,-0.095137,-0.035878,0.099474,-0.559115,0.377119,-0.205046);
            imageStore(outFeatures[4], ivec2(gl_GlobalInvocationID.xy), sum);
            sum = vec4(-0.275453, 0.006453, 0.130673, -0.041759);
            sum += f[0] * mat4(-0.092817,-0.262202,0.245622,0.795693,-0.155614,0.204832,-0.260413,-0.414593,-0.099770,0.142595,-0.069853,0.323335,-0.019192,0.021404,0.009176,0.403129);
            sum += f[1] * mat4(-0.181076,-0.018183,-0.013210,-0.711067,0.142907,-1.050550,0.039227,0.427149,-0.139330,-0.163606,-0.719481,-0.315952,0.057384,-0.023934,-0.035451,0.650445);
            sum += f[2] * mat4(-0.004336,0.380400,-0.007062,-0.042813,0.067652,0.063110,0.066941,0.099539,0.034265,0.366354,-0.002633,-0.002930,0.004102,0.011122,-0.063391,-0.003352);
            sum += f[3] * mat4(0.232721,0.092258,-0.048268,0.022033,0.069036,-0.044679,0.026402,0.035267,-0.328344,-0.020407,0.060973,-0.082098,0.032349,-0.004681,0.000914,-0.008575);
            sum += f[4] * mat4(0.022608,0.221902,-0.119972,0.078487,-0.140954,-0.002499,0.061206,-0.116346,-0.051660,0.039605,0.049101,0.081080,-0.022284,-0.004782,-0.002426,-0.008417);
            sum += f[5] * mat4(-0.228879,0.131226,-0.128516,-0.125672,0.116898,-0.023247,-0.254885,-0.048021,0.078682,-0.029388,-0.793624,0.012872,-0.019310,0.000216,-0.019680,0.020511);
            sum += f[6] * mat4(-0.221635,0.204660,-0.692134,0.178934,0.323970,-0.110115,-0.281358,-0.022299,-0.069334,0.002180,-0.267172,-0.101215,-0.006488,-0.011440,-0.012374,0.039206);
            sum += f[7] * mat4(-0.174199,-0.191543,0.379362,-0.183127,-0.029768,-0.281950,-0.249720,0.010807,0.011879,0.017899,-0.101904,0.056715,-0.006864,0.026076,0.001964,-0.030276);
            imageStore(outFeatures[5], ivec2(gl_GlobalInvocationID.xy), sum);
        }
    )