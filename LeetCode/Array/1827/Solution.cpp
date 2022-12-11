class Solution {
public:
    int minOperations(vector<int>& nums) {
        int add_num = 0;
        for(size_t i = 1; i < nums.size(); i++){
            if(nums[i]  <= nums[i-1]){
                add_num += nums[i-1] - nums[i] + 1;
                nums[i] = nums[i-1] + 1;
            }
        }
        return add_num;
    }
};