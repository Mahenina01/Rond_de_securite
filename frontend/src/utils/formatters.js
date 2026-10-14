export const formatPhone = (phone) => {
  if (!phone) return "";
  return phone.replace(/(\d{3})(?=\d)/g, "$1 ");
};

export const formatTime = (timestamp) => {
  if (!timestamp) return "";
  return timestamp;
};
