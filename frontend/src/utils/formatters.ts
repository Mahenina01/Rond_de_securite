export const formatPhone = (phone: string): string => {
    if (!phone) return "";
    return phone.replace(/(\d{3})(?=\d)/g, "$1 ")
};

export const formatTime = (timestamp: string): string =>{
    return timestamp;
}